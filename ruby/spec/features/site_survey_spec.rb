require 'spec_helper'

module Smith
  describe 'Wireless network site survey' do
    include ConfigHelper

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

    scenario 'ad-hoc network does not appear in results' do
      stub_iwlist_scan 'iwlist_scan_adhoc_mode_output.txt'
      
      visit '/wireless_networks'
      
      expect(page).to have_network('adskguest', 'Infrastructure', 'None')
      expect(page).not_to have_network(Smith::Config.adhoc_ssid, 'Ad-Hoc', 'None')
    end
  end
end
