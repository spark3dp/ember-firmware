module Smith
  module Config
    module Wireless
      extend System

      module_function

      def enable_managed_mode
        execute("wpa_action #{interface} stop")
        execute('service isc-dhcp-server stop')
        execute("ip addr flush dev #{interface}")
        execute("ifup #{interface}")
        true
      end

      def disconnect
        execute("wpa_cli -i #{interface} disconnect")
      end

      def enable_adhoc_mode
        execute("wpa_action #{interface} stop")
        execute("ip link set #{interface} up")
        execute("iwconfig #{interface} mode ad-hoc")
        execute("iwconfig #{interface} essid #{Config.adhoc_ssid}")
        execute("ip addr flush dev #{interface}")
        execute("ip addr add #{Config.adhoc_ip} brd + dev #{interface}")
        execute('service isc-dhcp-server start')
        true
      end

      def site_survey
        %x(iwlist #{interface} scan)
      end

      def interface
        Config.wireless_interface
      end

    end
  end
end
