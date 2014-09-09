require 'config_helper'

module Smith::Config
  describe WirelessNetwork do

    before do
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

    describe 'when saving as last configured', :tmp_dir do

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
