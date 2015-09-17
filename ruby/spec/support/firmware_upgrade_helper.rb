#  File: firmware_upgrade_helper.rb
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
