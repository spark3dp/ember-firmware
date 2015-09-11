#  File: wireless_network_spec.rb
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
  describe WirelessNetwork do

    include ConfigHelper

    before do
      use_in_memory_state
      allow(System).to receive(:wpa_psk).and_return('psk')
      allow(System).to receive(:nt_hash).and_return('pass')
    end

    subject(:network) { described_class.new(ssid: 'ssid', key: 'value', password: 'pass', passphrase: 'psk') }

    it 'behaves like an OpenStruct' do
      expect(network.key).to eq('value')
    end

    describe 'when instantiated with hash containing passphrase but no ssid' do
      it 'raises InvalidNetworkConfiguration' do
        expect { described_class.new('passphrase' => 'psk') }.to raise_error(InvalidNetworkConfiguration)
      end
    end

    describe 'when saving as last configured' do

      before { network.save_as_last_configured }
      let(:loaded) { described_class.last_configured }

      it 'does not persist the passphrase' do
        expect { loaded.passphrase }.to raise_error(InvalidNetworkConfiguration)
      end

      it 'does not persist the password' do
        expect { loaded.password }.to raise_error(InvalidNetworkConfiguration)
      end

      it 'does not persist the key' do
        expect { loaded.key }.to raise_error(InvalidNetworkConfiguration)
      end

    end

  end
end
