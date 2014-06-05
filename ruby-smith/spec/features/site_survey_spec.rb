require 'spec_helper'

module Smith
  describe 'Wireless network site survey' do
    scenario 'no wireless networks in range' do
      output = File.read(File.join(Smith.root, 'spec/resource/iwlist_scan_no_results_output.txt'))
      allow(Config::Wireless).to receive(:site_survey).and_return(output)

      visit '/wireless'

      expect(page).to have_content('No networks in range')
    end

    scenario 'wireless networks in range' do
      output = File.read(File.join(Smith.root, 'spec/resource/iwlist_scan_output.txt'))
      allow(Config::Wireless).to receive(:site_survey).and_return(output)

      visit '/wireless'

      expect(page).to have_content('adskguest')
      expect(page).to have_content(/none/i)
      expect(page).to have_content(/infrastructure/i)
      
      expect(page).to have_content('AutoGAL')
      expect(page).to have_content(/none/i)
      expect(page).to have_content(/infrastructure/i)
      
      expect(page).to have_content('WTA Wireless')
      expect(page).to have_content(/wpa-personal/i)
      expect(page).to have_content(/infrastructure/i)
      
      expect(page).to have_content('Autodesk')
      expect(page).to have_content(/wpa-enterprise/i)
    end
  end
end
