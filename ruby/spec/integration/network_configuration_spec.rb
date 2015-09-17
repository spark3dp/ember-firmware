#  File: network_configuration_spec.rb
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
  describe 'Network configuration', :tmp_dir do

    include ConfigHelper

    before { use_in_memory_state }
    
    scenario 'configure unsecured wireless network from file' do
      expect(WirelessInterface).to receive(:enable_managed_mode)

      CLI.start(['load', resource('unsecured.yml')])

      expect(wpa_roam_file_contents).to include_ssid('open_network')
      expect(wpa_roam_file_contents).to include_no_security
    end

    scenario 'configure wpa personal wireless network from file' do
      expect(WirelessInterface).to receive(:enable_managed_mode)
      allow(System).to receive(:wpa_psk).with('wpa_personal_network', 'personal_passphrase').and_return('hidden_psk')

      CLI.start(['load', resource('wpa_personal.yml')])

      expect(wpa_roam_file_contents).to include_ssid('wpa_personal_network')
      expect(wpa_roam_file_contents).to include_psk('hidden_psk')
    end

    scenario 'configure wpa enterprise wireless network from file' do
      expect(WirelessInterface).to receive(:enable_managed_mode)
      allow(System).to receive(:nt_hash).with('enterprise_pass').and_return('hash')

      CLI.start(['load', resource('wpa_enterprise.yml')])

      expect(wpa_roam_file_contents).to include_ssid('wpa_enterprise_network')
      expect(wpa_roam_file_contents).to include_eap_credentials('enterprise_user', 'hash', 'enterprise_domain')
    end

    scenario 'configure wep wireless network from file' do
      expect(WirelessInterface).to receive(:enable_managed_mode)

      CLI.start(['load', resource('wep.yml')])

      expect(wpa_roam_file_contents).to include_ssid('wep_network')
      expect(wpa_roam_file_contents).to include_wep_key('wep_key')
    end

    scenario 'configure unsecured wireless network from hash' do
      expect(WirelessInterface).to receive(:enable_managed_mode)

      Network.configure_from_hash(security: 'none', ssid: 'open_network')

      expect(wpa_roam_file_contents).to include_ssid('open_network')
      expect(wpa_roam_file_contents).to include_no_security
    end

    scenario 'attempt configuration with non-existent file' do
      expect { CLI.start(['load', 'foo']) }.to raise_error(Errno::ENOENT)
    end

    scenario 'attempt configuration with invalid security type' do
      expect { Network.configure_from_hash(security: 'other') }.to raise_error(InvalidNetworkConfiguration)
    end

    scenario 'attempt configuration with missing option' do
      expect { Network.configure_from_hash(security: 'none') }.to raise_error(InvalidNetworkConfiguration)
    end

  end
end
