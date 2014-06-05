require 'ostruct'

module Smith
  module Config
    module IwlistScanParser
      module_function

      def parse(iwlist_scan_output)
        reduce_cells(split_output(iwlist_scan_output).map { |cell| parse_cell(cell) })
      end

      def split_output(iwlist_scan_output)
        iwlist_scan_output.split(/Cell /m).drop(1)
      end

      def parse_cell(cell)
         information_elements =
           cell.scan(/IE: (.*?\n.*?\n.*?\n.*$)/).map { |groups| parse_information_element(groups.first) }
         encryption = cell.match(/Encryption key:(.*?)$/)[1]

        OpenStruct.new(
          ssid:                 cell.match(/ESSID:"(.*?)"$/)[1],
          mode:                 cell.match(/Mode:(.*?)$/)[1],
          encryption:           encryption,
          information_elements: information_elements,
          security:             get_security(encryption, information_elements)
        )
      end

      def parse_information_element(element)
        protocol =
          if element.match(/WPA\b/)
            'WPA'
          elsif element.match(/WPA2\b/)
            'WPA2'
          end

        OpenStruct.new(
          protocol:              protocol,
          group_cipher:          element.match(/Group Cipher : (.*?)$/)[1],
          pairwise_ciphers:      element.match(/Pairwise Ciphers.*?: (.*?)$/)[1].split(' '),
          authentication_suites: element.match(/Authentication Suites.*?: (.*?)$/)[1].split(' ')
        )
      end

      def reduce_cells(cells)
        cells.uniq { |c| c.ssid }
      end

      def get_security(encryption, information_elements)
        if encryption == 'on'
          ie = information_elements.first
          if ie.nil?
            'wep'
          elsif ie.authentication_suites.include?('802.1x')
            'wpa-enterprise'
          elsif ie.authentication_suites.include?('PSK')
            'wpa-personal'
          end
        else
          'none'
        end
      end

    end
  end
end
