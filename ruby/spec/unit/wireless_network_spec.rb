require 'spec_helper'
require 'ostruct'

module Smith::Config
  describe WirelessNetwork do

    subject(:network) { described_class.new(key: 'value', information_elements: [OpenStruct.new(key: 'val1'), OpenStruct.new(key: 'val2' )]) }

    it 'behaves like an OpenStruct' do
      expect(network.key).to eq('value')
      expect(network.information_elements).to eq([OpenStruct.new(key: 'val1'), OpenStruct.new(key: 'val2')])
    end

    describe 'when created from hash with nested array of information element hashes' do

      subject(:network) { described_class.new(information_elements: [{ key: 'val1' }, { key: 'val2' }]) }

      it 'constructs OpenStruct instances for each information element hash' do
        expect(network.information_elements).to eq([OpenStruct.new(key: 'val1'), OpenStruct.new(key: 'val2')])
      end

    end

    describe 'when created without information elements' do
      subject(:network) { described_class.new(key: 'val') }

      it 'initializes information elements field to empty array' do
        expect(network.information_elements).to eq([])
      end
    end

    describe 'when serializing' do
      
      it 'serializes information elements array' do
        expect(network.serialize[:information_elements]).to eq([{ key: 'val1' }, { key: 'val2' }])
      end

      it 'serializes top level keys' do
        expect(network.serialize[:key]).to eq('value')
      end

    end

  end
end
