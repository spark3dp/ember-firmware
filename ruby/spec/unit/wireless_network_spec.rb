require 'spec_helper'

module Smith::Config
  describe WirelessNetwork do

    subject(:network) { described_class.new(key: 'value') }

    it 'behaves like an OpenStruct' do
      expect(network.key).to eq('value')
    end

    describe '#encrypted?' do
      context 'when encryption is set to on' do
        it 'returns true' do
          network.encryption = 'on'
          expect(network.encrypted?).to eq(true)
        end
      end
      context 'when encryption is set to off' do
        it 'returns false' do
          network.encryption = 'off'
          expect(network.encrypted?).to eq(false)
        end
      end
    end

  end
end
