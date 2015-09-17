#  File: site_survey_spec.rb
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

require 'server_helper'

module Smith
  describe 'Wireless network site survey', :tmp_dir do
    include ConfigHelper

    before { use_in_memory_state }

    scenario 'no wireless networks in range' do
      stub_iwlist_scan 'iwlist_scan_no_results_output.txt'

      visit '/wireless_networks'

      expect(page).to have_content('No wireless networks in range')
    end

    scenario 'wireless networks in range' do
      stub_iwlist_scan 'iwlist_scan_output.txt'

      visit '/wireless_networks'

      expect(page).to have_network('adskguest', 'Infrastructure', 'None')
      expect(page).to have_network('AutoGAL', 'Infrastructure', 'None')
      expect(page).to have_network('WTA Wireless', 'Infrastructure', 'WPA Personal \(PSK\)')
      expect(page).to have_network('Autodesk', 'Infrastructure', 'WPA Enterprise \(EAP\)')
    end

    scenario 'network with hidden ssid does not appear in results' do
      stub_iwlist_scan 'iwlist_scan_hidden_ssid_output.txt'
      
      visit '/wireless_networks'
      
      expect(page).to have_content('No wireless networks in range')
    end
  end
end
