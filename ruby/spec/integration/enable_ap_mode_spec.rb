require 'spec_helper'

module Smith
	describe 'Network', :tmp_dir do

		let(:dhcpd_conf) { File.read(File.join(@tmp_path, 'dhcpd.conf')) }

		scenario 'AP mode enabled for wireless adapter' do
			ENV['AP_SSID'] = 'somessid'
			ENV['AP_IP'] = '192.168.5.254/24'
			
			expect(Config::Wireless).to receive(:enable_ap_mode)

			Config::Network.enable_ap_mode

			expect(File.read(File.join(@tmp_path, 'hostapd.conf'))).to include('ssid=somessid')
		
			expect(dhcpd_conf).to include('option subnet-mask 255.255.255.0;')
			expect(dhcpd_conf).to include('option broadcast-address 192.168.5.255;')
			expect(dhcpd_conf).to include('subnet 192.168.5.0 netmask 255.255.255.0 {')
			expect(dhcpd_conf).to include('range 192.168.5.5 192.168.5.150;')
			expect(dhcpd_conf).to include('option domain-name-servers 192.168.5.254;')
			expect(dhcpd_conf).to include('option routers 192.168.5.254;')
		end
	end
end
