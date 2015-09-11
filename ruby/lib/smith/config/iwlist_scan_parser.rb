#  File: iwlist_scan_parser.rb
#  Parses output of iwlist and generates corresponding WirelessNetwork instances
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
        information_elements = cell.split('IE: ').drop(1).map { |ie| parse_information_element(ie) }.compact
        encryption = cell.match(/Encryption key:(.*?)$/)[1]

        WirelessNetwork.new(
          ssid:                 cell.match(/ESSID:"(.*?)"$/)[1],
          mode:                 cell.match(/Mode:(.*?)$/)[1],
          encryption:           encryption,
          security:             get_security(encryption, information_elements)
        )
      end

      def parse_information_element(element)
        if element.match(/WPA\b/)
          protocol = 'WPA'
        elsif element.match(/WPA2\b/)
          protocol = 'WPA2'
        else
          return nil
        end

        OpenStruct.new(
          protocol:              protocol,
          group_cipher:          element.match(/Group Cipher : (.*?)$/)[1],
          pairwise_ciphers:      element.match(/Pairwise Ciphers.*?: (.*?)$/)[1].split(' '),
          authentication_suites: element.match(/Authentication Suites.*?: (.*?)$/)[1].split(' ')
        )
      end

      def reduce_cells(cells)
        cells.uniq { |c| c.ssid }.reject { |c| c.ssid.strip.empty? }
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
