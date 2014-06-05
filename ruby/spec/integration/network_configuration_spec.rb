require 'spec_helper'

module Smith::Config
describe 'Network configuration', :tmp_dir do

    before do
      ENV['WPA_ROAM_PATH'] = @tmp_path
      allow(Wired).to receive(:connected?).and_return(false)
    end

    let(:wpa_roam_file) { File.read(File.join(@tmp_path, 'wpa-roam.conf')) }

    def config_file(file_name)
      File.join(Smith.root, 'spec/resource', file_name)
    end

    scenario 'configure unsecured wireless network from file' do
      expect(Wireless).to receive(:enable_managed_mode)

      CLI.start(['load', config_file('unsecured.yml')])

      expect(wpa_roam_file).to include('ssid="open_network"')
      expect(wpa_roam_file).to include('key_mgmt=NONE')
    end

    scenario 'configure wpa personal wireless network from file' do
      expect(Wireless).to receive(:enable_managed_mode)

      CLI.start(['load', config_file('wpa_personal.yml')])

      expect(wpa_roam_file).to include('ssid="wpa_personal_network"')
      expect(wpa_roam_file).to include('psk="personal_passphrase"')
    end

    scenario 'configure wpa enterprise wireless network from file' do
      expect(Wireless).to receive(:enable_managed_mode)

      CLI.start(['load', config_file('wpa_enterprise.yml')])

      expect(wpa_roam_file).to include('ssid="wpa_enterprise_network"')
      expect(wpa_roam_file).to include('identity="enterprise_domain\enterprise_user"')
      expect(wpa_roam_file).to include('password="enterprise_pass"')
    end

    scenario 'configure unsecured wireless network from hash' do
      expect(Wireless).to receive(:enable_managed_mode)

      Network.configure(security: 'none', network_name: 'open_network')

      expect(wpa_roam_file).to include('ssid="open_network"')
      expect(wpa_roam_file).to include('key_mgmt=NONE')
    end

    scenario 'configure wireless network when wired interface is connected' do
      allow(Wired).to receive(:connected?).and_return(true)

      expect(Wireless).to receive(:enable_managed_mode)
      expect(Wireless).to receive(:disconnect)
      Network.configure(security: 'none', network_name: 'open_network')
    end

    scenario 'attempt configuration with non-existent file' do
      expect { CLI.start(['load', 'foo']) }.to raise_error(Errno::ENOENT)
      
    end

    scenario 'attempt configuration with invalid security type' do
      expect { Network.configure(security: 'other') }.to raise_error(InvalidNetworkConfiguration)
    end

    scenario 'attempt configuration with missing option' do
      expect { Network.configure(security: 'none') }.to raise_error(InvalidNetworkConfiguration)
    end

  end
end
