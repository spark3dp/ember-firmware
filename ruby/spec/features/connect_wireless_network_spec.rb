require 'server_helper'

module Smith
  describe 'Connect to wireless network', :tmp_dir do
    include ConfigHelper
    include PrintEngineHelper

    before do
      allow(Config::WiredInterface).to receive(:connected?).and_return(false)
      stub_iwlist_scan 'iwlist_scan_output.txt'
      visit '/wireless_networks'
    end

    context 'printer is in valid state to connect to wireless network and communication via command pipe is possible' do

      before do
        create_command_pipe
        create_printer_status_file
        open_command_pipe
      end

      after { close_command_pipe }

      scenario 'successfully connect to unsecured wireless network' do
        expect(Config::WirelessInterface).to receive(:enable_managed_mode)
        allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, true)
        
        within 'tr', text: 'adskguest' do
          click_button 'Connect'
        end
    
        wait_for_wireless_config

        expect(wpa_roam_file_contents).to include_ssid('adskguest')
        expect(wpa_roam_file_contents).to include_no_security
        expect(page).to have_content('Now attempting to connect to "adskguest"')
        expect(next_command_in_command_pipe).to eq(CMD_SHOW_WIRELESS_CONNECTING)
        expect(next_command_in_command_pipe).to eq(CMD_SHOW_WIRELESS_CONNECTED)
      end

      scenario 'successfully connect to wireless network secured with WPA personal' do
        expect(Config::WirelessInterface).to receive(:enable_managed_mode)
        allow(Config::System).to receive(:wpa_psk).with('WTA Wireless', 'personal_passphrase').and_return('hidden_psk')
        
        within 'tr', text: 'WTA Wireless' do
          click_button 'Connect'
        end

        fill_in 'Passphrase', with: 'personal_passphrase'
        click_button 'Connect'

        wait_for_wireless_config

        expect(wpa_roam_file_contents).to include_ssid('WTA Wireless')
        expect(wpa_roam_file_contents).to include_psk('hidden_psk')
        expect(page).to have_content('Now attempting to connect to "WTA Wireless"')
      end

      scenario 'successfully connect to wireless network secured with WPA enterprise' do
        expect(Config::WirelessInterface).to receive(:enable_managed_mode)
        allow(Config::System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')
        
        within 'tr', text: 'Autodesk' do
          click_button 'Connect'
        end
        
        fill_in 'Username', with: 'enterprise_user'
        fill_in 'Password', with: 'enterprise_pass'
        fill_in 'Domain', with: 'enterprise_domain'
        click_button 'Connect'

        wait_for_wireless_config
        
        expect(wpa_roam_file_contents).to include_ssid('Autodesk')
        expect(wpa_roam_file_contents).to include_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
        expect(page).to have_content('Now attempting to connect to "Autodesk"')
      end

      scenario 'successfully connect to wireless network secured with WEP' do
        expect(Config::WirelessInterface).to receive(:enable_managed_mode)
        
        within 'tr', text: 'testwifiwep' do
          click_button 'Connect'
        end
   
        fill_in 'Key', with: 'wep_key'
        click_button 'Connect'

        wait_for_wireless_config

        expect(wpa_roam_file_contents).to include_ssid('testwifiwep')
        expect(wpa_roam_file_contents).to include_wep_key('wep_key')
        expect(page).to have_content('Now attempting to connect to "testwifiwep"')
      end

      scenario 'attempt to connect to wireless network but connection is not established within timeout' do
        expect(Config::WirelessInterface).to receive(:enable_managed_mode)
        allow(Config::WirelessInterface).to receive(:connected?).and_return(false, false, false)
        
        within 'tr', text: 'adskguest' do
          click_button 'Connect'
        end
    
        wait_for_wireless_config

        expect(wpa_roam_file_contents).to include_ssid('adskguest')
        expect(wpa_roam_file_contents).to include_no_security
        expect(page).to have_content('Now attempting to connect to "adskguest"')
        expect(next_command_in_command_pipe).to eq(CMD_SHOW_WIRELESS_CONNECTING)
        expect(next_command_in_command_pipe).to eq(CMD_SHOW_WIRELESS_CONNECTION_FAILED)
      end

    end

    scenario 'attempt to connect to wireless network when printer is not in valid state'

    scenario 'attempt to connect to wireless network when communication with command pipe is not possible'

  end
end
