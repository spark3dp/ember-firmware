require 'spec_helper'

module Smith
  describe 'Connect to wireless network', :tmp_dir do
    include FileHelper
    include ConfigHelper

    wpa_roam_file_setup

    before do
      allow(Config::Wired).to receive(:connected?).and_return(false)
      stub_iwlist_scan 'iwlist_scan_output.txt'
    end

    before do
      visit '/wireless_networks'
    end

    scenario 'connect to unsecured wireless network' do
      expect(Config::Wireless).to receive(:enable_managed_mode)
      
      within 'tr', text: 'adskguest' do
	click_button 'Connect'
      end
  
      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('adskguest')
      expect(wpa_roam_file).to contain_no_security
      expect(page).to have_content('Now attempting to connect to "adskguest"')
    end

    scenario 'connect to wireless network secured with WPA personal' do
      expect(Config::Wireless).to receive(:enable_managed_mode)
      allow(Config::System).to receive(:wpa_psk).with('WTA Wireless', 'personal_passphrase').and_return('hidden_psk')
      
      within 'tr', text: 'WTA Wireless' do
	click_button 'Connect'
      end

      fill_in 'Passphrase', with: 'personal_passphrase'
      click_button 'Connect'

      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('WTA Wireless')
      expect(wpa_roam_file).to contain_psk('hidden_psk')
      expect(page).to have_content('Now attempting to connect to "WTA Wireless"')
    end

    scenario 'connect to wireless network secured with WPA enterprise' do
      expect(Config::Wireless).to receive(:enable_managed_mode)
      allow(Config::System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')
      
      within 'tr', text: 'Autodesk' do
	click_button 'Connect'
      end
      
      fill_in 'Username', with: 'enterprise_user'
      fill_in 'Password', with: 'enterprise_pass'
      fill_in 'Domain', with: 'enterprise_domain'
      click_button 'Connect'

      wait_for_wireless_config
      
      expect(wpa_roam_file).to contain_ssid('Autodesk')
      expect(wpa_roam_file).to contain_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
      expect(page).to have_content('Now attempting to connect to "Autodesk"')
    end

    scenario 'connect to wireless network secured with WEP' do
      expect(Config::Wireless).to receive(:enable_managed_mode)
      
      within 'tr', text: 'testwifiwep' do
        click_button 'Connect'
      end
 
      fill_in 'Key', with: 'wep_key'
      click_button 'Connect'

      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('testwifiwep')
      expect(wpa_roam_file).to contain_wep_key('wep_key')
      expect(page).to have_content('Now attempting to connect to "testwifiwep"')
    end

  end
end
