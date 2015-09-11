#  File: wireless_interface.rb
#  Functionality for interacting with a wireless network interface
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

require 'shellwords'

module Smith
  module Config
    module WirelessInterface

      extend System

      module_function

      def enable_managed_mode
        puts('Enabling managed mode')
        execute(%W(wpa_action #{name} stop))
        execute(%W(service dnsmasq restart))
        execute(%W(service hostapd stop))
        execute(%W(ip addr flush dev #{name}))
        execute(%W(ifup #{name}))
        puts('Managed mode enabled')
        true
      end

      def disconnect
        puts('Disconnecting wireless adapter')
        execute(%W(wpa_cli -i #{name} disconnect))
        puts('Wireless adapter disconnected')
      end

      def enable_ap_mode
        puts('Enabling AP mode')
        execute(%W(wpa_action #{name} stop))
        execute(%W(ip addr flush dev #{name}))
        execute(%W(ip addr add #{ap_ip} brd + dev #{name}))
        execute(%W(ip link set #{name} up))
        execute(%W(service dnsmasq restart))
        execute(%W(service hostapd restart))
        puts('AP mode enabled')
        true
      end

      def site_survey
        %x(iwlist #{Shellwords.shellescape(name)} scan)
      end

      def connected?
        !!(%x(wpa_cli -i #{Shellwords.shellescape(name)} status 2>&1) =~ /wpa_state=COMPLETED/)
      end

      def ap_mode_config
        @ap_mode_config ||= ApModeConfig.new(ap_ip, ap_ssid_prefix, name)
      end
      
      def ap_mode_config_binding
        ap_mode_config.get_binding
      end

      def ap_mode_ip_address
        ap_mode_config.ip_address
      end

      def name
        Settings.wireless_interface
      end

      def ap_ip
        Settings.ap_ip_address
      end

      def ap_ssid_prefix
        Settings.ap_ssid_prefix
      end

    end
  end
end
