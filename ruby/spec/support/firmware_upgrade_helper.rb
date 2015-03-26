# Setup for testing firmware upgrade

require 'fileutils'

module FirmwareUpgradeHelper
  
  # This module requires functionality from FileHelper
  # Automatically include FileHelper and set appropriate metadata on specs that use this
  def self.included(including_class)
    including_class.class_exec do

      let(:firmware_dir) { tmp_dir('firmware') }
      let(:firmware_versions_file) { File.join(firmware_dir, 'versions') }

      # Helper methods provided by this module require a temporary directory
      if metadata[:client]
        # Client specs are asynchronous and need the async tmp dir hooks
        metadata[:tmp_dir_async] = true
      else
        metadata[:tmp_dir] = true
        before { setup_firmware_upgrade }
      end
    end
  end

  def set_one_entry_versions_file
    FileUtils.copy(resource('versions-one_entry'), firmware_versions_file)
  end

  def set_two_entry_versions_file
    FileUtils.copy(resource('versions-two_entries'), firmware_versions_file)
  end

  def setup_firmware_upgrade
    Smith::Settings.firmware_dir = firmware_dir
    Smith::Settings.firmware_versions_file = firmware_versions_file
    FileUtils.mkdir(firmware_dir)
  end

end
