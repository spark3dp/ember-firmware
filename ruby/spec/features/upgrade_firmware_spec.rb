#  File: upgrade_firmware_spec.rb
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

require 'server_helper'

module Smith
  describe 'Firmware upgrade', :tmp_dir do

    include FirmwareUpgradeHelper

    let(:upgrade_package) { resource 'smith-0.0.2-valid.tar' }

    before { set_one_entry_versions_file }

    scenario 'submit form with valid upgrade package' do
      visit '/firmware_upgrade'
      attach_file 'Select firmware upgrade package', upgrade_package
      
      click_button 'Upgrade'
      
      expect(File.file?(File.join(firmware_dir, 'smith-0.0.2.img'))).to eq(true)
      expect(page).to have_content('Firmware upgraded successfully, please reboot printer')
    end

    scenario 'submit form without selecting file' do
      visit '/firmware_upgrade'
      click_button 'Upgrade'
      expect(page).to have_content('Please select a firmware upgrade package')
    end

    scenario 'submit form with invalid upgrade package' do
      visit '/firmware_upgrade'
      attach_file 'Select firmware upgrade package', resource('smith-0.0.2-invalid_checksum.tar')

      click_button 'Upgrade'
      
      expect(page).to have_content(/Unable to complete firmware upgrade/i)
    end

    scenario 'submit form with non-tar archive' do
      visit '/firmware_upgrade'
      attach_file 'Select firmware upgrade package', resource('print.tar.gz')
      click_button 'Upgrade'

      expect(page).to have_content('Upgrade package must be a tar archive')
    end

  end
end
