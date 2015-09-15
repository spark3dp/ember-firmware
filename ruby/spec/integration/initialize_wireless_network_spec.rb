#  File: initialize_wireless_network_spec.rb
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
require 'fileutils'

module Smith::Config
  describe 'Network', :tmp_dir do

    scenario 'initialize wireless network when wpa-roam config file exists' do
      FileUtils.touch(wpa_roam_file)

      expect(Network).not_to receive(:enable_ap_mode)

      CLI.start(['init'])
    end

    scenario 'initialize wireless network when wpa-roam config file does not exist' do
      expect(Network).to receive(:enable_ap_mode)

      CLI.start(['init'])
    end

  end
end
