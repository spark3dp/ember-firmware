require 'spec_helper'

module Configurator
  describe 'Network configuration', :tmp_dir do

    before do
      ENV['WPA_ROAM_PATH'] = @tmp_path
      ENV['WIRELESS_INTERFACE'] = 'wlan0'
    end

    let(:wpa_roam_file) { File.read(File.join(@tmp_path, 'wpa-roam.conf')) } 

    def config_file(file_name)
      File.join(Configurator.root, 'spec/resource', file_name)
    end

    scenario 'configure unsecured wireless network from file' do
      expect(Wireless).to receive(:enable_managed_mode).with('wlan0')
      
      Network.configure_from_file(config_file('unsecured.yml'))
      
      expect(wpa_roam_file).to include('ssid="open_network"')
      expect(wpa_roam_file).to include('key_mgmt=NONE')
    end

    scenario 'configure wpa personal wireless network from file' do
      expect(Wireless).to receive(:enable_managed_mode).with('wlan0')
      
      Network.configure_from_file(config_file('wpa_personal.yml'))
      
      expect(wpa_roam_file).to include('ssid="wpa_personal_network"')
      expect(wpa_roam_file).to include('psk="personal_passphrase"')
    end

    scenario 'configure wpa enterprise wireless network from file' do
      expect(Wireless).to receive(:enable_managed_mode).with('wlan0')
      
      Network.configure_from_file(config_file('wpa_enterprise.yml'))
      
      expect(wpa_roam_file).to include('ssid="wpa_enterprise_network"')
      expect(wpa_roam_file).to include('identity="enterprise_domain\enterprise_user"')
      expect(wpa_roam_file).to include('password="enterprise_pass"')
    end

    scenario 'configure unsecured wireless network from hash' do
      expect(Wireless).to receive(:enable_managed_mode).with('wlan0')
      
      Network.configure(security: 'none', network_name: 'open_network')
      
      expect(wpa_roam_file).to include('ssid="open_network"')
      expect(wpa_roam_file).to include('key_mgmt=NONE')
    end

    scenario 'attempt configuration with non-existent file' do
      expect { Network.configure_from_file('foo') }.to raise_error(Errno::ENOENT)
    end

    scenario 'attempt configuration with nil file' do
      expect { Network.configure_from_file(nil) }.to raise_error(RuntimeError)
    end

    scenario 'attempt configuration with invalid security type' do
      expect { Network.configure(security: 'other') }.to raise_error(InvalidNetworkConfiguration)
    end

    scenario 'attempt configuration with missing option' do
      expect { Network.configure(security: 'none') }.to raise_error(InvalidNetworkConfiguration)
    end

  end
end
