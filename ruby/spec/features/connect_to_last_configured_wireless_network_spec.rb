require 'spec_helper'

module Smith
  describe 'Connect to last configured wireless network', :tmp_dir do
    include FileHelper
    include ConfigHelper

    wpa_roam_file_setup

    before do
      allow(Config::WirelessInterface).to receive(:enable_managed_mode)
      allow(Config::WiredInterface).to receive(:connected?).and_return(false)

      stub_iwlist_scan 'iwlist_scan_output.txt'
      visit '/wireless_networks'
    end

    scenario 'connect to last (unsecured) wireless network' do
      # No previous network
      expect(page).not_to have_content('Last configured wireless network:')
      
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

      expect(page).to have_content('Last configured wireless network: adskguest')


      # The only connect button on the page is for the remembered wireless network
      expect(Config::WirelessInterface).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('adskguest')
      expect(wpa_roam_file).to contain_no_security
      expect(page).to have_content('Now attempting to connect to "adskguest"')
    end

    scenario 'connect to last (WPA personal) wireless network' do
      allow(Config::System).to receive(:wpa_psk).with('WTA Wireless', 'personal_passphrase').and_return('hidden_psk')
      
      # No previous network
      expect(page).not_to have_content('Last configured wireless network:')
      
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

      expect(page).to have_content('Last configured wireless network: WTA Wireless')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('passphrase').value).to be_nil
      fill_in 'Passphrase', with: 'personal_passphrase'
      
      expect(Config::WirelessInterface).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config
      
      expect(wpa_roam_file).to contain_ssid('WTA Wireless')
      expect(wpa_roam_file).to contain_psk('hidden_psk')
      expect(page).to have_content('Now attempting to connect to "WTA Wireless"')
    end

    scenario 'connect to last (WPA enterprise) wireless network' do
      allow(Config::System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')
      
      # No previous network
      expect(page).not_to have_content('Last configured wireless network:')
 
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

      expect(page).to have_content('Last configured wireless network: Autodesk')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('username').value).to eq('enterprise_user')
      expect(page.find_field('password').value).to be_nil
      expect(page.find_field('domain').value).to eq('enterprise_domain')
      
      fill_in 'Password', with: 'enterprise_pass'
      
      expect(Config::WirelessInterface).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config
      
      expect(wpa_roam_file).to contain_ssid('Autodesk')
      expect(wpa_roam_file).to contain_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
      expect(page).to have_content('Now attempting to connect to "Autodesk"')
    end

    scenario 'connect to last (WEP) wireless network' do
       # No previous network
      expect(page).not_to have_content('Last configured wireless network:')

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

      expect(page).to have_content('Last configured wireless network: testwifiwep')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('key').value).to be_nil
      fill_in 'Key', with: 'wep_key'
      
      expect(Config::WirelessInterface).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config

      expect(wpa_roam_file).to contain_ssid('testwifiwep')
      expect(wpa_roam_file).to contain_wep_key('wep_key')
      expect(page).to have_content('Now attempting to connect to "testwifiwep"')
    end

    scenario 'connect to last (secured) wireless network after loading from config file' do
      allow(Config::System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')
      
      # No previous network
      expect(page).not_to have_content('Last configured wireless network:')

      Config::CLI.start(['load', config_file('wpa_enterprise.yml')])

      # Simulate no networks in range
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'
      visit '/wireless_networks'
      
      # Assert the site survey is empty
      expect(page).to have_content('No wireless networks in range')

      expect(page).to have_content('Last configured wireless network: wpa_enterprise_network')

      # The only connect button on the page is for the remembered wireless network
      click_button 'Connect'

      expect(page.find_field('username').value).to eq('enterprise_user')
      expect(page.find_field('password').value).to be_nil
      expect(page.find_field('domain').value).to eq('enterprise_domain')
      
      fill_in 'Password', with: 'enterprise_pass'
      
      expect(Config::WirelessInterface).to receive(:enable_managed_mode)
      click_button 'Connect'
      wait_for_wireless_config
      
      expect(wpa_roam_file).to contain_ssid('wpa_enterprise_network')
      expect(wpa_roam_file).to contain_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
      expect(page).to have_content('Now attempting to connect to "wpa_enterprise_network"')
    end

  end
end
