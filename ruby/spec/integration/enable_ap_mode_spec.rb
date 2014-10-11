require 'config_helper'

module Smith::Config
  describe 'Network', :tmp_dir do

    before do
      Smith::Settings.ap_ssid_prefix = 'somessid'
      Smith::Settings.ap_ip_address = '192.168.5.254/24'
      Smith::Settings.wireless_interface = 'wlan4'
    end

    scenario 'AP mode enabled for wireless adapter' do
      expect(WirelessInterface).to receive(:enable_ap_mode)

      CLI.start(['mode', 'ap'])

      expect(hostapd_config_file_contents).to match(/ssid=somessid\s\S{6}$/)
      expect(hostapd_config_file_contents).to include('interface=wlan4')
    
      expect(dnsmasq_config_file_contents).to include('address=/#/192.168.5.254')
      expect(dnsmasq_config_file_contents).to include('dhcp-range=192.168.5.5,192.168.5.150,12h')
      expect(dnsmasq_config_file_contents).to include('interface=wlan4')
    end

    scenario 'AP mode enabled for wireless adapter when ssid has been generated before' do
      allow(WirelessInterface).to receive(:enable_ap_mode)
      
      CLI.start(['mode', 'ap'])

      ssid = /ssid=(.*?)$/.match(hostapd_config_file_contents)[1]

      CLI.start(['mode', 'ap'])

      expect(hostapd_config_file_contents).to match(/ssid=#{ssid}$/)
    end
  end
end
