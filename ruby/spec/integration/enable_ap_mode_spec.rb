require 'config_helper'

module Smith::Config
  describe 'Network', :tmp_dir do

    let(:dnsmasq_conf) { File.read(tmp_dir('dnsmasq.conf')) }

    def hostapd_conf
      File.read(tmp_dir('hostapd.conf'))
    end

    before do
      ENV['AP_SSID_PREFIX'] = 'somessid'
      ENV['AP_IP'] = '192.168.5.254/24'
      ENV['WIRELESS_INTERFACE'] = 'wlan0'
    end

    scenario 'AP mode enabled for wireless adapter' do
      expect(WirelessInterface).to receive(:enable_ap_mode)

      CLI.start(['mode', 'ap'])

      expect(conf = hostapd_conf).to match(/ssid=somessid\s\S{6}$/)
      expect(conf).to include('interface=wlan0')
    
      expect(dnsmasq_conf).to include('address=/#/192.168.5.254')
      expect(dnsmasq_conf).to include('dhcp-range=192.168.5.5,192.168.5.150,12h')
      expect(dnsmasq_conf).to include('interface=wlan0')
    end

    scenario 'AP mode enabled for wireless adapter when ssid has been generated before' do
      allow(WirelessInterface).to receive(:enable_ap_mode)
      
      CLI.start(['mode', 'ap'])

      ssid = /ssid=(.*?)$/.match(hostapd_conf)[1]

      CLI.start(['mode', 'ap'])

      expect(hostapd_conf).to match(/ssid=#{ssid}$/)
    end
  end
end
