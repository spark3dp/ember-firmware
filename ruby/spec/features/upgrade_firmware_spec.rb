require 'server_helper'

module Smith
  describe 'Firmware upgrade', :tmp_dir do
    let(:upgrade_package) { resource 'smith-0.0.2-valid.tar' }

    # Stub all calls to Firmware
    # Test cases may make override this with more specific stub configurations
    before { allow(Config::Firmware).to receive(:upgrade) }

    scenario 'submit form with valid upgrade package' do
      visit '/firmware_upgrade'
      attach_file 'Select firmware upgrade package', upgrade_package
      
      uploaded_file = nil
      allow(Config::Firmware).to receive(:upgrade) { |arg| uploaded_file = arg }
      
      click_button 'Upgrade'

      expect(File.read(uploaded_file)).to eq(File.read(upgrade_package))
      expect(page).to have_content('Firmware upgraded successfully, please reboot printer')
    end

    scenario 'submit form without selecting file' do
      visit '/firmware_upgrade'
      click_button 'Upgrade'
      expect(page).to have_content('Please select a firmware upgrade package')
    end

    scenario 'submit form with invalid upgrade package' do
      visit '/firmware_upgrade'
      attach_file 'Select firmware upgrade package', upgrade_package

      allow(Config::Firmware).to receive(:upgrade).and_raise(Config::Firmware::UpgradeError, 'the error message')
      
      click_button 'Upgrade'
      
      expect(page).to have_content('Unable to complete firmware upgrade (the error message')
    end

    scenario 'submit form with non-tar archive' do
      visit '/firmware_upgrade'
      attach_file 'Select firmware upgrade package', resource('print.tar.gz')
      click_button 'Upgrade'

      expect(page).to have_content('Upgrade package must be a tar archive')
    end

  end
end
