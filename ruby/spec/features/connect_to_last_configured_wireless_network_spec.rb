require 'spec_helper'

module Smith
  describe 'Connect to last configured wireless network', :tmp_dir do
    include FileHelper
    include ConfigHelper

    wpa_roam_file_setup

    before do
      allow(Config::Wireless).to receive(:enable_managed_mode)
      allow(Config::Wired).to receive(:connected?).and_return(false)

      stub_iwlist_scan 'iwlist_scan_output.txt'
      visit '/wireless_networks'
    end

    scenario 'connect to last (unsecured) wireless network' do
      # No previous network
      expect(page).not_to have_content('Last selected wireless network:')
      
      # Connect
      within 'tr', text: 'adskguest' do
	click_button 'Connect'
      end
      wait_for_wireless_config

      # Simulate no networks in range
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'
      visit '/wireless_networks'
      
      # Assert the site survey is empty
      expect(page).to have_content('No wireless networks in range')

      expect(page).to have_content('Last selected wireless network:')
      expect(page).to have_network('adskguest', 'Infrastructure', 'None')


      # The only connect button on the page is for the remembered wireless network
      expect(Config::Wireless).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('adskguest')
      expect(wpa_roam_file).to contain_no_security
      expect(page).to have_content('Now attempting to connect to "adskguest"')
    end

    scenario 'connect to last (WPA personal) wireless network' do
       # No previous network
      expect(page).not_to have_content('Last selected wireless network:')
      
      within 'tr', text: 'WTA Wireless' do
	click_button 'Connect'
      end

      fill_in 'Passphrase', with: 'personal_passphrase'
      
      click_button 'Connect'
      wait_for_wireless_config

      # Simulate no networks in range
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'
      visit '/wireless_networks'
      
      # Assert the site survey is empty
      expect(page).to have_content('No wireless networks in range')

      expect(page).to have_content('Last selected wireless network:')
      expect(page).to have_network('WTA Wireless', 'Infrastructure', 'WPA Personal \(PSK\)')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('passphrase').value).to eq('personal_passphrase')
      
      expect(Config::Wireless).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config
      
      expect(wpa_roam_file).to contain_ssid('WTA Wireless')
      expect(wpa_roam_file).to contain_psk('personal_passphrase')
      expect(page).to have_content('Now attempting to connect to "WTA Wireless"')
    end

    scenario 'connect to last (WPA enterprise) wireless network' do
      # No previous network
      expect(page).not_to have_content('Last selected wireless network:')
 
      within 'tr', text: 'Autodesk' do
	click_button 'Connect'
      end
      
      fill_in 'Username', with: 'enterprise_user'
      fill_in 'Password', with: 'enterprise_pass'
      fill_in 'Domain', with: 'enterprise_domain'
      
      click_button 'Connect'
      wait_for_wireless_config
      
      # Simulate no networks in range
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'
      visit '/wireless_networks'
      
      # Assert the site survey is empty
      expect(page).to have_content('No wireless networks in range')

      expect(page).to have_content('Last selected wireless network:')
      expect(page).to have_network('Autodesk', 'Infrastructure', 'WPA Enterprise \(EAP\)')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('username').value).to eq('enterprise_user')
      expect(page.find_field('password').value).to eq('enterprise_pass')
      expect(page.find_field('domain').value).to eq('enterprise_domain')
      
      expect(Config::Wireless).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config
      
      expect(wpa_roam_file).to contain_ssid('Autodesk')
      expect(wpa_roam_file).to contain_eap_credentials('enterprise_user', 'enterprise_pass', 'enterprise_domain')
      expect(page).to have_content('Now attempting to connect to "Autodesk"')
    end

    scenario 'connect to last (WEP) wireless network' do
       # No previous network
      expect(page).not_to have_content('Last selected wireless network:')

      within 'tr', text: 'testwifiwep' do
        click_button 'Connect'
      end
 
      fill_in 'Key', with: 'wep_key'
      
      click_button 'Connect'
      wait_for_wireless_config

      # Simulate no networks in range
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'
      visit '/wireless_networks'
      
      # Assert the site survey is empty
      expect(page).to have_content('No wireless networks in range')

      expect(page).to have_content('Last selected wireless network:')
      expect(page).to have_network('testwifiwep', 'Infrastructure', 'WEP')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('key').value).to eq('wep_key')
      
      expect(Config::Wireless).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('testwifiwep')
      expect(wpa_roam_file).to contain_wep_key('wep_key')
      expect(page).to have_content('Now attempting to connect to "testwifiwep"')
    end

  end
end
