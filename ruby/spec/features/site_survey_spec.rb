require 'spec_helper'

module Smith
  describe 'Wireless network site survey' do
    scenario 'no wireless networks in range' do
      output = File.read(File.join(Smith.root, 'spec/resource/iwlist_scan_no_results_output.txt'))
      allow(Config::Wireless).to receive(:site_survey).and_return(output)

      visit '/wireless_networks'

      expect(page).to have_content('No wireless networks in range')
    end

    scenario 'wireless networks in range' do
      output = File.read(File.join(Smith.root, 'spec/resource/iwlist_scan_output.txt'))
      allow(Config::Wireless).to receive(:site_survey).and_return(output)

      visit '/wireless_networks'

      expect(page).to have_network('adskguest', 'Infrastructure', 'None')
      expect(page).to have_network('AutoGAL', 'Infrastructure', 'None')
      expect(page).to have_network('WTA Wireless', 'Infrastructure', 'WPA Personal \(PSK\)')
      expect(page).to have_network('Autodesk', 'Infrastructure', 'WPA Enterprise \(EAP\)')
    end
  end
end
