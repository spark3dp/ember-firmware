#  File: connect_to_last_configured_wireless_network_spec.rb
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
  describe 'Connect to last configured wireless network', :tmp_dir do
    include ConfigHelper
    include PrintEngineHelper

    before do
      use_in_memory_state
      allow(Config::WirelessInterface).to receive(:enable_managed_mode)
      allow(Config::WirelessInterface).to receive(:connected?).and_return(true)

      stub_iwlist_scan 'iwlist_scan_output.txt'
      visit '/wireless_networks'

      create_command_pipe
      create_printer_status_file
      set_printer_status(state: HOME_STATE, ui_sub_state: NO_SUBSTATE)
      open_command_pipe
    end

    after { close_command_pipe }

    scenario 'connect to last (unsecured) wireless network' do
      # No previous network
      expect(page).not_to have_content('Last configured wireless network:')
      
      # Connect
      within 'tr', text: 'adskguest' do
        click_button 'Connect'
      end
      assert_connecting_connected_commands_sent

      # Simulate no networks in range
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'
      visit '/wireless_networks'
      
      # Assert the site survey is empty
      expect(page).to have_content('No wireless networks in range')

      expect(page).to have_content('Last configured wireless network: adskguest')


      # The only connect button on the page is for the remembered wireless network
      expect(Config::WirelessInterface).to receive(:enable_managed_mode)
      click_button 'Connect'
      assert_connecting_connected_commands_sent

      expect(wpa_roam_file_contents).to include_ssid('adskguest')
      expect(wpa_roam_file_contents).to include_no_security
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
      assert_connecting_connected_commands_sent

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
      assert_connecting_connected_commands_sent
      
      expect(wpa_roam_file_contents).to include_ssid('WTA Wireless')
      expect(wpa_roam_file_contents).to include_psk('hidden_psk')
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
      assert_connecting_connected_commands_sent
      
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
      assert_connecting_connected_commands_sent
      
      expect(wpa_roam_file_contents).to include_ssid('Autodesk')
      expect(wpa_roam_file_contents).to include_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
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
      assert_connecting_connected_commands_sent

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
      assert_connecting_connected_commands_sent

      expect(wpa_roam_file_contents).to include_ssid('testwifiwep')
      expect(wpa_roam_file_contents).to include_wep_key('wep_key')
      expect(page).to have_content('Now attempting to connect to "testwifiwep"')
    end

    scenario 'connect to last (secured) wireless network after loading from config file' do
      allow(Config::System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')
      
      # No previous network
      expect(page).not_to have_content('Last configured wireless network:')

      Config::CLI.start(['load', resource('wpa_enterprise.yml')])

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
      assert_connecting_connected_commands_sent
      
      expect(wpa_roam_file_contents).to include_ssid('wpa_enterprise_network')
      expect(wpa_roam_file_contents).to include_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
      expect(page).to have_content('Now attempting to connect to "wpa_enterprise_network"')
    end

  end
end
