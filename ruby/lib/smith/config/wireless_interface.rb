module Smith
  module Config
    module WirelessInterface

      extend System

      module_function

      def enable_managed_mode
        execute("wpa_action #{name} stop")
        execute('service dnsmasq restart')
        execute('service hostapd stop')
        execute("ip addr flush dev #{name}")
        execute("ifup #{name}")
        true
      end

      def disconnect
        execute("wpa_cli -i #{name} disconnect")
      end

      def enable_ap_mode
        execute("wpa_action #{name} stop")
        execute("ip addr flush dev #{name}")
        execute("ip addr add #{ap_ip} brd + dev #{name}")
        execute("ip link set #{name} up")
        execute('service dnsmasq restart')
        execute('service hostapd restart')
        true
      end

      def site_survey
        %x(iwlist #{name} scan)
      end

      def name
        ENV['WIRELESS_INTERFACE']
      end

      def ap_mode_config
        @ap_mode_config ||= ApModeConfig.new(ap_ip, ap_ssid, name)
      end
      
      def ap_mode_config_binding
        ap_mode_config.get_binding
      end

      def ap_mode_ip_address
        ap_mode_config.ip_address
      end

      def ap_ip
        ENV['AP_IP']
      end

      def ap_ssid
        ENV['AP_SSID']
      end

    end
  end
end
