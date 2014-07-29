require 'spec_helper'

module Smith::Config
  describe 'Network', :tmp_dir do

    scenario 'managed mode enabled for wireless adapter when adapter is in ap mode' do
      ENV['AP_SSID_PREFIX'] = 'somessid'
      ENV['AP_IP'] = '192.168.5.254/24'
      ENV['WIRELESS_INTERFACE'] = 'wlan0'

      allow(WirelessInterface).to receive(:enable_ap_mode)
      expect(WirelessInterface).to receive(:enable_managed_mode)
      
      CLI.start(['mode', 'ap'])
      CLI.start(['mode', 'managed'])
    end

  end
end
