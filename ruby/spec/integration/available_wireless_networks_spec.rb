require 'spec_helper'

module Smith::Config
  describe 'Wireless network site survey' do
    include ConfigHelper

    scenario 'query site for available networks' do
      stub_iwlist_scan 'iwlist_scan_output.txt'

      networks = Smith::Config.available_wireless_networks
      expect(networks.length).to eq(5)

      autodesk = networks.select { |n| n.ssid == 'Autodesk' }.first
      expect(autodesk.security).to eq('wpa-enterprise')
      expect(autodesk.mode).to eq('Master')
      
      autogal = networks.select { |n| n.ssid == 'AutoGAL' }.first
      expect(autogal.security).to eq('none')
      expect(autogal.mode).to eq('Master')
      
      adskguest = networks.select { |n| n.ssid == 'adskguest' }.first
      expect(adskguest.security).to eq('none')
      expect(adskguest.mode).to eq('Master')

      wta_wireless = networks.select { |n| n.ssid == 'WTA Wireless' }.first
      expect(wta_wireless.security).to eq('wpa-personal')
      expect(wta_wireless.mode).to eq('Master')
      
      testwifiwep = networks.select { |n| n.ssid == 'testwifiwep' }.first
      expect(testwifiwep.security).to eq('wep')
      expect(testwifiwep.mode).to eq('Master')
    end
  end
end
