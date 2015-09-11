#  File: enable_managed_mode_spec.rb
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
