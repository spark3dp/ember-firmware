require 'config_helper'

module Smith::Config
  describe 'Network', :tmp_dir do

    include ConfigHelper

    before { use_in_memory_state }

    scenario 'managed mode enabled for wireless adapter when adapter is in ap mode' do
      Smith::Settings.ap_ssid_prefix = 'somessid'
      Smith::Settings.ap_ip_address = '192.168.5.254/24'
      Smith::Settings.wireless_interface = 'wlan4'

      allow(WirelessInterface).to receive(:enable_ap_mode)
      expect(WirelessInterface).to receive(:enable_managed_mode)
      
      CLI.start(['mode', 'ap'])
      CLI.start(['mode', 'managed'])
    end

  end
end
