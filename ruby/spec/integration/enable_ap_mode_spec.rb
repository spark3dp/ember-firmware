require 'spec_helper'

module Smith::Config
  describe 'Network', :tmp_dir do

    let(:dnsmasq_conf) { File.read(tmp_dir('dnsmasq.conf')) }
    let(:hostapd_conf) { File.read(tmp_dir('hostapd.conf')) }

    scenario 'AP mode enabled for wireless adapter' do
      ENV['AP_SSID'] = 'somessid'
      ENV['AP_IP'] = '192.168.5.254/24'
      ENV['WIRELESS_INTERFACE'] = 'wlan0'
      
      expect(WirelessInterface).to receive(:enable_ap_mode)

      CLI.start(['mode', 'ap'])

      expect(hostapd_conf).to include('ssid=somessid')
      expect(hostapd_conf).to include('interface=wlan0')
    
      expect(dnsmasq_conf).to include('address=/#/192.168.5.254')
      expect(dnsmasq_conf).to include('dhcp-range=192.168.5.5,192.168.5.150,12h')
      expect(dnsmasq_conf).to include('interface=wlan0')
    end
  end
end
