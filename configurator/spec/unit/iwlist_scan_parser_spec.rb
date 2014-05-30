require 'spec_helper'

module Configurator
  describe IwlistScanParser do

    let(:iwlist_output) { File.read(File.join(Configurator.root, 'spec/resource/iwlist_scan_output.txt')) }

    def assert_field(field, expected)
      expect(described_class.parse(iwlist_output).map { |n| n.send(field) }).to eq(expected)
    end

    context 'when parsing output of iwlist scan' do
      it 'parses ssid' do
        assert_field(:ssid, ['adskguest', 'AutoGAL', 'WTA Wireless', 'Autodesk', 'Autodesk', 'AutoGAL', 'adskguest', 'Autodesk', 'AutoGAL'])
      end

      it 'parses mode' do
        assert_field(:mode, ['Master', 'Master', 'Master', 'Master', 'Master', 'Master', 'Master', 'Master', 'Master'])
      end
    end
  end
end