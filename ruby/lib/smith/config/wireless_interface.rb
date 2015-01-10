module Smith
  module Config
    module WirelessInterface

      extend System

      module_function

      def enable_managed_mode
        puts('Enabling managed mode')
        execute("wpa_action #{name} stop")
        execute('service dnsmasq restart')
        execute('service hostapd stop')
        execute("ip addr flush dev #{name}")
        execute("ifup #{name}")
        puts('Managed mode enabled')
        true
      end

      def disconnect
        puts('Disconnecting wireless adapter')
        execute("wpa_cli -i #{name} disconnect")
        puts('Wireless adapter disconnected')
      end

      def enable_ap_mode
        puts('Enabling AP mode')
        execute("wpa_action #{name} stop")
        execute("ip addr flush dev #{name}")
        execute("ip addr add #{ap_ip} brd + dev #{name}")
        execute("ip link set #{name} up")
        execute('service dnsmasq restart')
        execute('service hostapd restart')
        puts('AP mode enabled')
        true
      end

      def site_survey
        %x(iwlist #{name} scan)
      end

      def connected?
        !!(%x(wpa_cli -i #{name} status 2>&1) =~ /wpa_state=COMPLETED/)
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
