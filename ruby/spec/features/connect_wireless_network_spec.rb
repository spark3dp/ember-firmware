require 'spec_helper'

module Smith
  describe 'Connect to wireless network', :tmp_dir do
    extend WpaRoamFileHelper

    wpa_roam_file_setup

    before do
      allow(Config::Wireless).to receive(:site_survey).and_return(File.read(File.join(Smith.root, 'spec/resource/iwlist_scan_output.txt')))
    end

    scenario 'connect to unsecured wireless network' do
      visit '/wireless_networks'

      expect(Config::Wireless).to receive(:enable_managed_mode)

      within 'tr', text: 'adskguest' do
	click_button 'Connect'
      end
   
      # Find the thread performing the configuration and wait for it to complete 
      Thread.list.reject { |thread| thread == Thread.main }.first.join

      expect(wpa_roam_file).to contain_ssid('adskguest')
      expect(wpa_roam_file).to contain_no_security

      expect(page).to have_content('Now attempting to connect to "adskguest"')
    end
  end
end
