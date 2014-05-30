require 'ostruct'

module Configurator
  module IwlistScanParser
    module_function

    def parse(iwlist_scan_output)
		  iwlist_scan_output.split(/Cell /m).drop(1).map { |cell| parse_cell(cell) }
    end

    def parse_cell(cell)
      values = {}

      cell.split("\n").each do |line|
        if m = line.match(/ESSID:"(.*)"/)
          values[:ssid] = m[1]
        elsif m = line.match(/Mode:(.*)/)
          values[:mode] = m[1]
        end
      end
      OpenStruct.new(values)
    end

  end
end