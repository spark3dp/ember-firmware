require 'spec_helper'

module Smith
  describe 'Connect to wireless network', :tmp_dir do
    extend WpaRoamFileHelper

    wpa_roam_file_setup

    def wait_for_config
      # Find the thread performing the configuration and wait for it to complete 
      Thread.list.reject { |thread| thread == Thread.main }.first.join
    end

    before do
      allow(Config::Wireless).to receive(:site_survey).and_return(File.read(File.join(Smith.root, 'spec/resource/iwlist_scan_output.txt')))
      expect(Config::Wireless).to receive(:enable_managed_mode)
      visit '/wireless_networks'
    end

    scenario 'connect to unsecured wireless network' do
      within 'tr', text: 'adskguest' do
	click_button 'Connect'
      end
  
      wait_for_config

      expect(wpa_roam_file).to contain_ssid('adskguest')
      expect(wpa_roam_file).to contain_no_security
      expect(page).to have_content('Now attempting to connect to "adskguest"')
    end

    scenario 'connect to wireless network secured with WPA personal' do
      within 'tr', text: 'WTA Wireless' do
	click_button 'Connect'
      end

      fill_in 'Passphrase', with: 'personal_passphrase'
      click_button 'Connect'

      wait_for_config

      expect(wpa_roam_file).to contain_ssid('WTA Wireless')
      expect(wpa_roam_file).to contain_psk('personal_passphrase')
      expect(page).to have_content('Now attempting to connect to "WTA Wireless"')
    end

    scenario 'connect to wireless network secured with WPA enterprise' do
      within 'tr', text: 'Autodesk' do
	click_button 'Connect'
      end
      
      fill_in 'Username', with: 'enterprise_user'
      fill_in 'Password', with: 'enterprise_pass'
      fill_in 'Domain', with: 'enterprise_domain'
      click_button 'Connect'

      wait_for_config
      
      expect(wpa_roam_file).to contain_ssid('Autodesk')
      expect(wpa_roam_file).to contain_eap_credentials('enterprise_user', 'enterprise_pass', 'enterprise_domain')
      expect(page).to have_content('Now attempting to connect to "Autodesk"')
    end

    scenario 'connect to wireless network secured with WEP' do
      within 'tr', text: 'testwifiwep' do
	click_button 'Connect'
      end
 
      fill_in 'Key', with: 'wep_key'
      click_button 'Connect'

      wait_for_config

      expect(wpa_roam_file).to contain_ssid('testwifiwep')
      expect(wpa_roam_file).to contain_wep_key('wep_key')
      expect(page).to have_content('Now attempting to connect to "testwifiwep"')
    end

  end
end
