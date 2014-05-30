module Configurator
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

    def enable_adhoc_mode(ip, ssid)
      execute("wpa_action #{interface} stop")
      execute("ip link set #{interface} up")
      execute("iwconfig #{interface} mode ad-hoc")
      execute("iwconfig #{interface} essid #{ssid}")
      execute("ip addr add #{ip} brd + dev #{interface}")
      execute('service isc-dhcp-server start')
      true
    end

    def interface
      Configurator.wireless_interface
    end

  end
end
