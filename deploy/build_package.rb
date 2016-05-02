#!/usr/bin/env ruby

require 'fileutils'
require 'open-uri'
require 'optparse'

# Parse arguments
options = {}

OptionParser.new do |opts|

  opts.on('--version VERSION', 'Specify version string to use in package name') do |version|
    options[:version] = version
  end

  opts.on('--[no-]automate',
          'Assume time is correct, remove existing file systems, and generate a new file system') do |automate|
    options[:automate] = automate
  end

  opts.on('--smith-binary SMITH_BINARY',
          'Path to smith binary to incorporate into build') do |smith_bin|
    options[:smith_bin] = smith_bin
  end

  opts.on('--gem-cache-dir CACHE_DIR',
          'Path to directory containing smith Ruby gem and dependencies to incorporate into build') do |gem_cache_dir|
    options[:gem_cache_dir] = gem_cache_dir
  end

  opts.on('--zee-binary ZEE_BINARY',
          'Path to zee binary to incorporate into build') do |zee_bin|
    options[:zee_bin] = zee_bin
  end

end.parse!

# Read version from provided argument or from smith gem
if options[:version]
  version = options[:version]
else
  require 'smith/version'
  version = Smith::VERSION
end

# Get the location of this script
# This is the absolute location of the deploy directory
ROOT = File.expand_path('..', __FILE__)

# Script-level configuration variables
DEPLOY_DIR             = File.join(ROOT, 'deploy')
FIRMWARE_SETUP_DIR     = File.join(ROOT, 'setup', 'main', 'firmware')
MD5SUM_TEMP_FILE       = File.join(ROOT, 'md5sum')
VERSIONS_FILE_NAME     = 'versions'
SCRIPT_DIR             = File.join(ROOT, 'build_scripts')
INSTALL_SCRIPT_NAME    = 'install.sh'
CLONE_OIB_SCRIPT_NAME  = 'clone_oib.sh'
CONFIGS_DIR            = File.join(ROOT, 'configs')
OIB_COMMON_CONFIG_FILE = File.join(CONFIGS_DIR, 'smith-common.conf')
OIB_CONFIG_FILE        = File.join(CONFIGS_DIR, 'smith-release.conf')
# OIB_CONFIG_FILE and OIB_COMMON_CONFIG_FILE are concatenated and the result is written to OIB_TEMP_CONFIG_FILE
OIB_TEMP_CONFIG_FILE   = File.join(CONFIGS_DIR, '.smith-release.conf')
OIB_PROJECT_FILE       = File.join(ROOT, '.project')
VAR_CONTENTS_DIR_NAME  = 'var_contents'


# Add color methods to string
class String
  # colorization
  def colorize(color_code)
    "\e[#{color_code}m#{self}\e[0m"
  end

  def red
    colorize(31)
  end

  def green
    colorize(32)
  end

  def yellow
    colorize(33)
  end
end

# Function definitions
def check_for_squashfs_tools
  %x(which mksquashfs)
  abort "squashfs-tools required, install with 'apt-get install squashfs-tools', aborting".red unless $?.to_i == 0
end

def check_for_internet
  open('http://ftp.us.debian.org/debian/', read_timeout: 5)
rescue
  abort 'Could not reach ftp.us.debian.org, check internet connectivity, aborting'.red
end

def ensure_last_command_success(cmd)
  abort "Could not execute '#{cmd}' successfully, aborting".red unless $?.to_i == 0
end

def check_date
  print "The system date is #{%x(date).sub("\n", '')} Is this correct? [Y/n]: ".yellow
  if $stdin.gets.sub("\n", '').downcase == 'n'
    abort 'Set the date and run again, aborting'.red
  end
  print "\n"
end

def generate_md5sum_file(input_file, output_file)
  md5sum_cmd = %Q(cd "#{File.dirname(input_file)}" && md5sum "#{File.basename(input_file)}")
  md5sum_line = %x(#{md5sum_cmd})
  ensure_last_command_success(md5sum_cmd)
  File.write(output_file, md5sum_line)
  puts 'Operation complete'.green
  print "\n"
end

def run_command(cmd)
  # Append 2>&1 to redirect stderr to stdout since #system throws away stderr
  system("#{cmd} 2>&1")
  ensure_last_command_success(cmd)
  puts 'Operation complete'.green
  print "\n"
end

def build_filesystem(redirect_output_to_log)
  check_for_internet
  
  abort "#{OIB_COMMON_CONFIG_FILE} does not exist, aborting".red unless File.file?(OIB_COMMON_CONFIG_FILE)
  abort "#{OIB_CONFIG_FILE} does not exist, aborting".red unless File.file?(OIB_CONFIG_FILE)
  # Concatenate the common config options with the release specific options
  File.write(OIB_TEMP_CONFIG_FILE, "#{File.read(OIB_COMMON_CONFIG_FILE)}\n#{File.read(OIB_CONFIG_FILE)}")

  # Clone/pull omap-image-builder
  puts "Cloning/pulling omap-image-builder".green
  run_command(%Q("#{File.join(SCRIPT_DIR, CLONE_OIB_SCRIPT_NAME)}"))

  # Call to omap-image-builder
  if redirect_output_to_log
    puts "Executing omap-image-builder for #{OIB_CONFIG_FILE}  See #{File.join(ROOT, 'oib.log')} for output".green
    oib_cmd = %Q(cd "#{ROOT}" && omap-image-builder/RootStock-NG.sh -c #{OIB_TEMP_CONFIG_FILE} > oib.log)
  else
    puts "Executing omap-image-builder for #{OIB_CONFIG_FILE}".green
    oib_cmd = %Q(cd "#{ROOT}" && omap-image-builder/RootStock-NG.sh -c #{OIB_TEMP_CONFIG_FILE})
  end

  run_command(oib_cmd)

  # rename the generated var directory and create an empty mount point; we mount non-upgradeable storage to /var
  export_filename = %x(bash -e -c 'source #{OIB_PROJECT_FILE}; echo -n $export_filename')
  abort "unable to read export_filename from #{OIB_PROJECT_FILE}, aborting".red unless $?.to_i == 0
  filesystem_root = Dir[File.join(DEPLOY_DIR, export_filename, '/*')].grep(/rootfs/).first
  var_dir = File.join(filesystem_root, 'var')
  FileUtils.mv(var_dir, File.join(filesystem_root, VAR_CONTENTS_DIR_NAME))
  Dir.mkdir(var_dir)
end

def prompt_to_build_new_filesystem
  print 'Do you want to build a new base filesystem with omap-image-builder? [y/N]: '.yellow
  if $stdin.gets.sub("\n", '').downcase == 'y'
    print "\n"
    build_filesystem(true)
  else
    print "\n"
  end
end

def prompt_for_filesystem_index(filesystems)
  count = filesystems.length
  print "Select a base filesystem [0-#{count - 1}]: ".yellow
  input = $stdin.gets.sub("\n", '')
  index = input.to_i

  if input !~ /\A\d+?\z/ || index < 0 || index > count - 1
    puts "Selection must be 0 to #{count - 1}".red
    prompt_for_filesystem_index(filesystems)
  else
    print "\n"
    index
  end
end

def list_filesystems(filesystems)
  puts "Found #{filesystems.length} existing filesystem(s):"
  print "\n"

  filesystems.each_with_index do |f, i|
    puts "\t [#{i}] #{f}"
  end
  print "\n"
end

def get_filesystems
# Get a list of filesystems sorted from newest to oldest
# Skip any non-directories
Dir[File.join(DEPLOY_DIR, '/*')].
  grep(/smith-release/).
  reject { |d| !File.directory?(d) }.
  sort_by { |d| File.stat(d).mtime }.
  reverse
end

# Remove intermediate files if they exist any time the script exits
at_exit do
  File.delete(OIB_TEMP_CONFIG_FILE) if File.exist?(OIB_TEMP_CONFIG_FILE)
  File.delete(MD5SUM_TEMP_FILE) if File.exist?(MD5SUM_TEMP_FILE)
  FileUtils.rm_r(Dir[File.join(ROOT, '*.img')])
end

# Handle ctrl+c cleanly
trap 'INT' do
  print "\n"
  abort 'aborting'.red
end

# Begin execution
puts 'Smith firmware image builder script'
print "\n"
check_for_squashfs_tools

if options[:automate]
  puts "The system date is #{%x(date).sub("\n", '')}".green
  print "\n"
  
  puts 'Removing all existing filesystems'.green
  FileUtils.rm_r(get_filesystems)
  puts 'Operation complete'.green
  print "\n"
  
  build_filesystem(false)
else
  check_date
  prompt_to_build_new_filesystem
end

filesystems = get_filesystems.map { |d| d.sub("#{DEPLOY_DIR}/", '') }

abort 'No base filesystem found, at least one filesystem must exist to continue, aborting'.red if filesystems.empty?

if options[:automate]
  selected_filesystem = filesystems.first
else
  list_filesystems(filesystems)
  selected_filesystem = filesystems[prompt_for_filesystem_index(filesystems)]
end

selected_filesystem_root = Dir[File.join(DEPLOY_DIR, selected_filesystem, '/*')].grep(/rootfs/).first

abort 'The selection does not contain a rootfs directory, aborting'.red if selected_filesystem_root.nil?

puts "Building version #{version}".green
print "\n"

image_temp_file = File.join(ROOT, "smith-#{version}.img")
package_name = File.join(DEPLOY_DIR, "smith-#{version}.tar")

puts 'Running install script'.green
# Pass the install script the absolute path to the selected_filesystem_root
# Also pass through optional paths to core components
run_command(%Q("#{File.join(SCRIPT_DIR, INSTALL_SCRIPT_NAME)}" "#{selected_filesystem_root}" "#{options[:smith_bin]}" "#{options[:gem_cache_dir]}" "#{options[:zee_bin]}"))

puts "Building squashfs image (#{File.basename(image_temp_file)}) with #{selected_filesystem}".green
# Remove any existing files; mksquashfs will attempt to append if the file exists
FileUtils.rm_r(image_temp_file) if File.exist?(image_temp_file)
run_command(%Q(mksquashfs "#{selected_filesystem_root}" "#{image_temp_file}" -e #{VAR_CONTENTS_DIR_NAME} boot))

puts 'Generating md5sum file'.green
generate_md5sum_file(image_temp_file, MD5SUM_TEMP_FILE)

puts 'Building package'.green
run_command(%Q(cd "#{ROOT}" && tar vcf "#{package_name}" "#{File.basename(image_temp_file)}" "#{File.basename(MD5SUM_TEMP_FILE)}"))

if !options[:automate]
  puts 'Updating setup firmware'.green
  FileUtils.mkdir_p(FIRMWARE_SETUP_DIR)
  run_command(%Q(rm -rfv "#{FIRMWARE_SETUP_DIR}"/* && cp -v "#{image_temp_file}" "#{FIRMWARE_SETUP_DIR}" && cp -v "#{MD5SUM_TEMP_FILE}" "#{FIRMWARE_SETUP_DIR}/#{VERSIONS_FILE_NAME}"))
end

puts "Successfully built #{package_name}, size: #{File.size(package_name) / 1048576}M".green
