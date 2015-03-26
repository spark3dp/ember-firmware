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
