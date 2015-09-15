#  File: network.rb
#  Functionality for configuring network adapter(s)
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

require 'yaml'
require 'erb'
require 'fileutils'

module Smith
  module Config
    module Network
      module_function

      def configure(wireless_network)
        File.write(Settings.wpa_roam_file, ERB.new(wireless_network.wpa_roam_template).result(wireless_network.get_binding))
        wireless_network.save_as_last_configured
        WirelessInterface.enable_managed_mode
      end

      def configure_from_hash(hash)
        configure(WirelessNetwork.new(hash))
      end

      def configure_from_file(file_path)
        configure_from_hash(YAML.load_file(file_path))
      end

      def available_wireless_networks
        IwlistScanParser.parse(WirelessInterface.site_survey)
      end

      def enable_ap_mode
        File.write(Settings.hostapd_config_file, ERB.new(Config.get_template('hostapd.conf.erb')).result(WirelessInterface.ap_mode_config_binding))
        File.write(Settings.dnsmasq_config_file, ERB.new(Config.get_template('dnsmasq.conf.erb')).result(WirelessInterface.ap_mode_config_binding))
        WirelessInterface.enable_ap_mode
      end

      def enter_managed_mode(wireless_network)
        configure(wireless_network)
        Timeout::timeout(Settings.wireless_connection_timeout) do
          loop do
            break if Config::WirelessInterface.connected?
            sleep Settings.wireless_connection_poll_interval
          end
        end
      rescue Timeout::Error
        enable_ap_mode
        File.delete(Settings.wpa_roam_file)
        Printer.show_wireless_connection_failed
      rescue StandardError => e
        $stderr.puts(e.inspect + "\n" + e.backtrace.map{ |l| l.prepend('      ') }.join("\n"))
      else
        Printer.show_wireless_connected
      end

      def init
        enable_ap_mode unless File.file?(Settings.wpa_roam_file)
      end

    end
  end
end
