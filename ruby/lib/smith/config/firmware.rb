#  File: firmware.rb
#  Functionality for upgrading the firmware from an upgrade package
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'rubygems/package'
require 'zlib'
require 'digest/md5'
require 'fileutils'

module Smith
  module Config
    class Firmware
      class UpgradeError < StandardError; end

      CHECKSUM_FILE = 'md5sum'

      class << self
        def upgrade(package_path)
          new(package_path).upgrade
        end
      end

      # Params:
      # package_path: path to upgrade package containing the new firmware
      def initialize(package_path)
        @package_path = package_path
      end

      # The function that performs the upgrade
      def upgrade
        # Cleanup to remove any directories/files that might conflict with extraction
        cleanup_firmware_directory

        # Extract package into firmware directory
        extract_package

        # Verify that the md5sum file is contained in the package
        md5sum_contents = read_md5_file
        
        # Verify the md5 checksum
        verify_image(*md5sum_contents.split)
        
        # Get the last entry in the current versions file, it becomes the backup entry
        backup_entry = File.readlines(Settings.firmware_versions_file).last

        # Write the temporary versions file with the upgraded image as the primary entry
        # The last entry from the current versions file becomes the backup entry
        temp_versions_file = File.join(Settings.firmware_dir, 'new_versions')
        File.open(temp_versions_file, 'w') do |f|
          f.write("#{backup_entry}#{md5sum_contents}")
          # Call fsync to make sure the changes are flushed to disk
          f.fsync
        end

        # Rename the temporary versions file to the actual versions file
        # This is the atomic update; the system will continue to boot with the old entries until this rename is executed
        File.rename(temp_versions_file, Settings.firmware_versions_file)

        File.open(Settings.firmware_versions_file) do |f|
          # Call fsync to make sure the changes are flushed to disk
          f.fsync
        end

      ensure
        # Always cleanup to prevent accumulation of unnecessary files
        cleanup_firmware_directory
      end

      private

      # Verify the md5 checksum of a file
      # Raises an +UpgradeError+ if the checksum of specified file does
      # not match the specified checksum value of if the image file does not exist
      # Params:
      # md5_checksum: expected checksum value
      # image_name: name of image file being checked
      def verify_image(md5_checksum, image_name)
        path = File.join(Settings.firmware_dir, image_name)
        raise(UpgradeError, "expected image #{path.inspect} not found in upgrade package (#{@package_path})") unless File.file?(path)
        raise(UpgradeError, "checksum of new firmware image #{path.inspect} is invalid") unless Digest::MD5.hexdigest(File.read(path)) == md5_checksum
      end

      # Read the name of the image file and its md5 checksum from
      # a checksum file that is expected to contain the output from the
      # md5sum utility and be enclosed in the upgrade package
      #
      # The contents of the file is returned as a String if the format is valid and the file exists
      # Otherwise an UpgradeError is raised
      def read_md5_file
        path = File.join(Settings.firmware_dir, CHECKSUM_FILE)
        
        raise(UpgradeError, "md5sum file not found in upgrade package (#{@package_path})") unless File.file?(path)
        
        contents = File.read(path)

        # Format is 32 characters (checksum) followed by whitespace followed by the image name
        unless contents =~ /^\S{32}?\s+?\S+?$/
          raise UpgradeError, 'no properly formatted MD5 checksum lines found in md5sum file'
        end

        contents
      end

      # Remove everything other than the backup and primary images and the versions file
      def cleanup_firmware_directory
        entries = File.readlines(Settings.firmware_versions_file)
        keep_list = [
          Settings.firmware_versions_file,
          File.join(Settings.firmware_dir, entries.first.split.last),
          File.join(Settings.firmware_dir, entries.last.split.last)
        ]
        FileUtils.rm_r(Dir[File.join(Settings.firmware_dir, '*')].reject { |e| keep_list.include? e })
      end

      # Extract gzipped tar archive at package path into the firmware directory
      def extract_package
        tar_extract = Gem::Package::TarReader.new(File.open(@package_path))
        tar_extract.rewind
        
        tar_extract.each do |entry|
          # Ignore any non-file entries that are contained in the archive
          if entry.file?
            # Raise exception if an existing file has the same name (don't overwrite existing image files)
            destination_path = File.join(Settings.firmware_dir, entry.full_name)
            raise(UpgradeError, "error extracting #{@package_path}, the file #{destination_path.inspect} already exists") if File.exist?(destination_path)
            File.open(destination_path, 'w') do |f|
              f.write(entry.read)
              # Call fsync to make sure the changes are flushed to disk
              f.fsync
            end
          end
        end

      end

    end
  end
end
