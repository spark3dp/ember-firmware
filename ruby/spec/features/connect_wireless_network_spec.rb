#  File: connect_wireless_network_spec.rb
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
require 'fileutils'

module Smith
  describe 'Connect to wireless network', :tmp_dir do
    include ConfigHelper
    include PrintEngineHelper

    before do
      use_in_memory_state
      create_printer_status_file
      stub_iwlist_scan 'iwlist_scan_output.txt'
      visit '/wireless_networks'
    end

    context 'printer is in valid state to connect to wireless network' do

      before { set_printer_status(state: HOME_STATE, ui_sub_state: NO_SUBSTATE) }
      
      context 'communication via command pipe is possible' do

        before do
          create_command_pipe
          open_command_pipe
        end

        after { close_command_pipe }

        scenario 'successfully connect to unsecured wireless network' do
          expect(Config::WirelessInterface).to receive(:enable_managed_mode)
          allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, true)
          
          within 'tr', text: 'adskguest' do
            click_button 'Connect'
          end
      
          assert_connecting_connected_commands_sent
          expect(wpa_roam_file_contents).to include_ssid('adskguest')
          expect(wpa_roam_file_contents).to include_no_security
          expect(page).to have_content('Now attempting to connect to "adskguest"')
        end

        scenario 'successfully connect to wireless network secured with WPA personal' do
          expect(Config::WirelessInterface).to receive(:enable_managed_mode)
          allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, true)
          allow(Config::System).to receive(:wpa_psk).with('WTA Wireless', 'personal_passphrase').and_return('hidden_psk')
          
          within 'tr', text: 'WTA Wireless' do
            click_button 'Connect'
          end

          fill_in 'Passphrase', with: 'personal_passphrase'
          click_button 'Connect'

          assert_connecting_connected_commands_sent
          expect(wpa_roam_file_contents).to include_ssid('WTA Wireless')
          expect(wpa_roam_file_contents).to include_psk('hidden_psk')
          expect(page).to have_content('Now attempting to connect to "WTA Wireless"')
        end

        scenario 'successfully connect to wireless network secured with WPA enterprise' do
          expect(Config::WirelessInterface).to receive(:enable_managed_mode)
          allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, true)
          allow(Config::System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')
          
          within 'tr', text: 'Autodesk' do
            click_button 'Connect'
          end
          
          fill_in 'Username', with: 'enterprise_user'
          fill_in 'Password', with: 'enterprise_pass'
          fill_in 'Domain', with: 'enterprise_domain'
          click_button 'Connect'

          assert_connecting_connected_commands_sent
          expect(wpa_roam_file_contents).to include_ssid('Autodesk')
          expect(wpa_roam_file_contents).to include_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
          expect(page).to have_content('Now attempting to connect to "Autodesk"')
        end

        scenario 'successfully connect to wireless network secured with WEP' do
          expect(Config::WirelessInterface).to receive(:enable_managed_mode)
          allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, true)
          
          within 'tr', text: 'testwifiwep' do
            click_button 'Connect'
          end
     
          fill_in 'Key', with: 'wep_key'
          click_button 'Connect'

          assert_connecting_connected_commands_sent
          expect(wpa_roam_file_contents).to include_ssid('testwifiwep')
          expect(wpa_roam_file_contents).to include_wep_key('wep_key')
          expect(page).to have_content('Now attempting to connect to "testwifiwep"')
        end

        scenario 'attempt to connect to wireless network but connection is not established within timeout' do
          # Create a config file to ensure that the config is deleted after failure to connect
          FileUtils.touch(wpa_roam_file)

          expect(Config::WirelessInterface).to receive(:enable_managed_mode)
          expect(Config::Network).to receive(:enable_ap_mode)
          allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, false)
          
          within 'tr', text: 'adskguest' do
            click_button 'Connect'
          end
      
          expect(next_command_in_command_pipe).to eq(CMD_SHOW_WIRELESS_CONNECTING)
          expect(next_command_in_command_pipe).to eq(CMD_SHOW_WIRELESS_CONNECTION_FAILED)
          expect(page).to have_content('Now attempting to connect to "adskguest"')
          expect(File.file?(wpa_roam_file)).to eq(false)
        end

      end

      scenario 'attempt to connect to wireless network when communication with command pipe is not possible' do

        within 'tr', text: 'adskguest' do
          click_button 'Connect'
        end

        expect(page).to have_content /unable to send command/i
      end

    end

    scenario 'attempt to connect to wireless network when printer is not in valid state' do
      set_printer_status(state: PRINTING_STATE, ui_sub_state: NO_SUBSTATE)
      
      within 'tr', text: 'adskguest' do
        click_button 'Connect'
      end
      
      expect(page).to have_content /printer state .*? invalid/i
    end

  end
end
