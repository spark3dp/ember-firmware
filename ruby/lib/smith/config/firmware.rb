require 'rubygems/package'
require 'zlib'
require 'digest/md5'
require 'fileutils'

module Smith
  module Config
    module Firmware
      class UpgradeError < StandardError; end

      module_function

      def upgrade(package_path)
        # Cleanup to remove any directories/files that might confilict with extraction
        cleanup_firmware_directory

        dir = extract_package(package_path)
        md5sum_contents = read_md5_file(dir)
        md5_checksum, image_name = md5sum_contents.split
        verify_image(File.join(dir, image_name), md5_checksum)
        
        # Move the image file into the firmware directory
        FileUtils.move(File.join(dir, image_name), Settings.firmware_dir)

        # Get the last entry in the current versions file, it becomes the backup entry
        backup_entry = File.readlines(Settings.firmware_versions_file).last

        # Write the temporary versions file with the upgraded image as the primary entry
        # The last entry from the current versions file becomes the backup entry
        temp_versions_file = File.join(dir, 'new_versions')
        File.write(temp_versions_file, "#{backup_entry}#{md5sum_contents}")

        # Rename the temporary versions file to the actual versions file
        # This is the atomic update; the system will continue to boot with the old entries until this rename is executed
        File.rename(temp_versions_file, Settings.firmware_versions_file)

      ensure
        # Always cleanup to prevent accumulation of unnecessary files
        cleanup_firmware_directory
      end

      def verify_image(path, md5_checksum)
        if Digest::MD5.hexdigest(File.read(path)) != md5_checksum
          raise UpgradeError, "Checksum of new firmware image (#{path}) is invalid"
        end
      end

      def read_md5_file(dir)
        # This provides the name of the image file and its md5 checksum
        
        contents = File.read(File.join(dir, 'md5sum'))

        # Format is 32 characters (checksum) followed by whitespace followed by the image name
        unless contents =~ /^\S{32}?\s+?\S+?$/
          raise UpgradeError, "No properly formatted MD5 checksum lines found in md5sum file, looked in #{dir}"
        end

        contents
      end

      def cleanup_firmware_directory
        # Remove everything other than the backup and primary images and the versions file
        entries = File.readlines(Settings.firmware_versions_file)
        keep_list = [
          Settings.firmware_versions_file,
          File.join(Settings.firmware_dir, entries.first.split.last),
          File.join(Settings.firmware_dir, entries.last.split.last)
        ]
        FileUtils.rm_r(Dir[File.join(Settings.firmware_dir, '*')].reject { |e| keep_list.include? e })
      end

      def extract_package(path)
        # Extract specified archive into a directory based on the file basename
        tar_extract = Gem::Package::TarReader.new(File.open(path))
        tar_extract.rewind
        
        dir = File.join(Settings.firmware_dir, 'contents')
        FileUtils.mkdir(dir)

        tar_extract.each do |entry|
          # Ignore any directories that are contained in the archive
          if entry.file?
            File.write(File.join(dir, entry.full_name), entry.read)
          end
        end

        dir
      end

    end
  end
end
