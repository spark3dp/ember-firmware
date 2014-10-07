#!/usr/bin/env ruby

require 'fileutils'

# Read version from provided argument or from smith gem
if ARGV[0]
  version = ARGV.shift
else
  require 'smith/version'
  version = Smith::VERSION
end

# Get the location of this script
# This is the absolute location of the deploy directory
root = File.expand_path('..', __FILE__)

# Script-level configuration variables
deploy_dir = File.join(root, 'deploy')
firmware_setup_dir = File.join(root, 'setup', 'main', 'firmware')
md5sum_temp_file = File.join(root, 'md5sum')
versions_file_name = 'versions'
script_dir = File.join(root, 'build_package_scripts')
install_script_name = 'install.sh'

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

def prompt_to_build_new_filesystem(root)
  print 'Do you want to build a new base filesystem with omap-image-builder? [y/N]: '.yellow
  if $stdin.gets.sub("\n", '').downcase == 'y'
    print "\n"
    # Call to omap-image-builder
    puts "Executing omap-image-builder for configs/smith-release.conf  See #{File.join(root, 'oib.log')} for output".green
    oib_cmd = %Q(cd "#{root}" && omap-image-builder/RootStock-NG.sh -c configs/smith-release.conf > oib.log 2>&1)
    %x(#{oib_cmd})
    ensure_last_command_success(oib_cmd)
    puts 'done'.green
  end
  print "\n"
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

# Remove intermediate files if they exist any time the script exits
at_exit do
  File.delete(md5sum_temp_file) if File.exist?(md5sum_temp_file)
  FileUtils.rm_r(Dir[File.join(root, '*.img')])
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
check_date
prompt_to_build_new_filesystem(root)

# Get a list of filesystems sorted from newest to oldest
# Skip any non-directories
filesystems = Dir[File.join(deploy_dir, '/*')].
  grep(/smith-release/).
  reject { |d| !File.directory?(d) }.
  sort_by { |d| File.stat(d).mtime }.
  reverse.
  map { |d| d.sub("#{deploy_dir}/", '') }

abort 'No base filesystem found, at least one filesystem must exist to continue, aborting'.red if filesystems.empty?

list_filesystems(filesystems)
selected_filesystem = filesystems[prompt_for_filesystem_index(filesystems)]
selected_filesystem_root = Dir[File.join(deploy_dir, selected_filesystem, '/*')].grep(/rootfs/).first

abort 'The selection does not contain a rootfs directory, aborting'.red if selected_filesystem_root.nil?

print "\n"
puts "Building version #{version}".green
print "\n"

image_temp_file = File.join(root, "smith-#{version}.img")
package_name = File.join(deploy_dir, "smith-#{version}.tar")

puts 'Running install script'.green
# Pass the install script the absolute path to the selected_filesystem_root
run_command(%Q("#{File.join(script_dir, install_script_name)}" "#{selected_filesystem_root}"))

puts "Building squashfs image (#{File.basename(image_temp_file)}) with #{selected_filesystem}".green
run_command(%Q(mksquashfs "#{selected_filesystem_root}" "#{image_temp_file}" -e var_contents boot))

puts 'Generating md5sum file'.green
generate_md5sum_file(image_temp_file, md5sum_temp_file)

puts 'Building package'.green
run_command(%Q(cd "#{root}" && tar vcf "#{package_name}" "#{File.basename(image_temp_file)}" "#{File.basename(md5sum_temp_file)}"))

puts 'Updating setup firmware'.green
FileUtils.mkdir_p(firmware_setup_dir)
run_command(%Q(rm -rfv "#{firmware_setup_dir}"/* && cp -v "#{image_temp_file}" "#{firmware_setup_dir}" && cp -v "#{md5sum_temp_file}" "#{firmware_setup_dir}/#{versions_file_name}"))

puts "Successfully built #{package_name}, size: #{File.size(package_name) / 1048576}M".green
