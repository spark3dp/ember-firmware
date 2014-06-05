require 'spec_helper'

module Smith::Config
  describe IwlistScanParser do

    def read_resource(name)
      File.read(File.join(Smith.root, 'spec/resource', name))
    end

    context 'when parsing output' do
      it 'returns an array with elements for each cell in the specified iwlist scan output' do
        split_output = described_class.split_output(read_resource('iwlist_scan_output.txt'))

        expect(split_output.length).to eq(9)
        expect(split_output.first).to eq(read_resource('iwlist_scan_cell_open.txt'))
      end
    end

    context 'when parsing cell for network with no security' do
      it 'returns object with fields corrsponding to relevant cell information' do
        cell = described_class.parse_cell(read_resource('iwlist_scan_cell_open.txt'))

        expect(cell.ssid).to eq('adskguest')
        expect(cell.mode).to eq('Master')
        expect(cell.encryption).to eq('off')
        expect(cell.information_elements).to be_empty
        expect(cell.security).to eq('none')
      end
    end

    context 'when parsing cell for network with wep security' do
      it 'returns object with fields corrsponding to relevant cell information' do
        cell = described_class.parse_cell(read_resource('iwlist_scan_cell_wep.txt'))

        expect(cell.ssid).to eq('testwifiwep')
        expect(cell.mode).to eq('Master')
        expect(cell.encryption).to eq('on')
        expect(cell.information_elements).to be_empty
        expect(cell.security).to eq('wep')
      end
    end

    context 'when parsing cell for network with wpa2 enterprise security' do
      it 'returns object with fields corrsponding to relevant cell information' do
        cell = described_class.parse_cell(read_resource('iwlist_scan_cell_wpa2_enterprise.txt'))

        expect(cell.ssid).to eq('Autodesk')
        expect(cell.mode).to eq('Master')
        expect(cell.encryption).to eq('on')
        expect(cell.security).to eq('wpa-enterprise')

        expect(cell.information_elements[0].protocol).to eq('WPA2')
        expect(cell.information_elements[0].group_cipher).to eq('CCMP')
        expect(cell.information_elements[0].pairwise_ciphers).to match_array(['CCMP'])
        expect(cell.information_elements[0].authentication_suites).to match_array(['802.1x'])
      end
    end
    
    context 'when parsing cell for network with wpa2 personal security' do
      it 'returns object with fields corrsponding to relevant cell information' do
        cell = described_class.parse_cell(read_resource('iwlist_scan_cell_wpa2_personal.txt'))

        expect(cell.ssid).to eq('testwifiwpa')
        expect(cell.mode).to eq('Master')
        expect(cell.encryption).to eq('on')
        expect(cell.security).to eq('wpa-personal')

        expect(cell.information_elements[0].protocol).to eq('WPA2')
        expect(cell.information_elements[0].group_cipher).to eq('CCMP')
        expect(cell.information_elements[0].pairwise_ciphers).to match_array(['CCMP'])
        expect(cell.information_elements[0].authentication_suites).to match_array(['PSK'])
      end
    end
    
    context 'when parsing cell for network with wpa personal security' do
      it 'returns object with fields corrsponding to relevant cell information' do
        cell = described_class.parse_cell(read_resource('iwlist_scan_cell_wpa_personal.txt'))

        expect(cell.ssid).to eq('testwifiwpa')
        expect(cell.mode).to eq('Master')
        expect(cell.encryption).to eq('on')
        expect(cell.security).to eq('wpa-personal')

        expect(cell.information_elements[0].protocol).to eq('WPA')
        expect(cell.information_elements[0].group_cipher).to eq('TKIP')
        expect(cell.information_elements[0].pairwise_ciphers).to match_array(['TKIP'])
        expect(cell.information_elements[0].authentication_suites).to match_array(['PSK'])
      end
    end
    
    context 'when parsing cell for network with wpa & wpa2 personal security' do
      it 'returns object with fields corrsponding to relevant cell information' do
        cell = described_class.parse_cell(read_resource('iwlist_scan_cell_wpa_both_personal.txt'))

        expect(cell.ssid).to eq('testwifiwpa')
        expect(cell.mode).to eq('Master')
        expect(cell.encryption).to eq('on')
        expect(cell.security).to eq('wpa-personal')

        expect(cell.information_elements[0].protocol).to eq('WPA')
        expect(cell.information_elements[0].group_cipher).to eq('TKIP')
        expect(cell.information_elements[0].pairwise_ciphers).to match_array(['CCMP', 'TKIP'])
        expect(cell.information_elements[0].authentication_suites).to match_array(['PSK'])

        expect(cell.information_elements[1].protocol).to eq('WPA2')
        expect(cell.information_elements[1].group_cipher).to eq('TKIP')
        expect(cell.information_elements[1].pairwise_ciphers).to match_array(['CCMP', 'TKIP'])
        expect(cell.information_elements[1].authentication_suites).to match_array(['PSK'])
      end
    end

    context 'when reducing array of parsed cells' do
      it 'removes cells with duplicate ssids' do
        cells = [
          OpenStruct.new(ssid: 'net1', information_elements: [OpenStruct.new(protocol: 'WPA')]),
          OpenStruct.new(ssid: 'net1', information_elements: [OpenStruct.new(protocol: 'WPA2')]),
          OpenStruct.new(ssid: 'net2', information_elements: [OpenStruct.new(protocol: 'WPA')])
        ]
        expect(described_class.reduce_cells(cells).length).to eq(2)
      end
    end
  end
end
