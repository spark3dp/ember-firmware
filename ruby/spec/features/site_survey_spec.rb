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
