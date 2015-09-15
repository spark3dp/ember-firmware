#  File: enable_ap_mode_spec.rb
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

require 'config_helper'

module Smith::Config
  describe 'Network', :tmp_dir do

    include ConfigHelper

    before do
      use_in_memory_state
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
