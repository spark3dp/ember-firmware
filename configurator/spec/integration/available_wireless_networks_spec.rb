require 'spec_helper'

module Configurator
  describe 'Wireless network site survey' do

    let(:iwlist_scan_output) { File.read(File.join(Configurator.root, 'spec/resource/iwlist_scan_output.txt')) }

    scenario 'query site for available networks' do
      allow(Wireless).to receive(:site_survey).and_return(iwlist_scan_output)

      expect(Configurator.available_wireless_networks).to match_array([
        OpenStruct.new(ssid: 'Autodesk',     type: 'infrastructure', security: 'wpa-enterprise'),
        OpenStruct.new(ssid: 'AutoGAL',      type: 'infrastructure', security: 'none'),
        OpenStruct.new(ssid: 'adskguest',    type: 'infrastructure', security: 'none'),
        OpenStruct.new(ssid: 'WTA Wireless', type: 'infrastructure', security: 'wpa-personal')
      ])
    end
  end
end