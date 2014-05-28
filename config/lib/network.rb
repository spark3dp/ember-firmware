require 'open3'

module Network
  module_function

  def enable_managed_mode(interface)
    execute('service isc-dhcp-server stop')
    execute("ip addr flush dev #{interface}")
    execute("ifup #{interface}")
    true
  end

  def enable_adhoc_mode(interface, ip, ssid)
    execute("wpa_action #{interface} stop")
    execute("ip link set #{interface} up")
    execute("iwconfig #{interface} mode ad-hoc")
    execute("iwconfig #{interface} essid #{ssid}")
    execute("ip addr add #{ip} brd + dev #{interface}")
    execute('service isc-dhcp-server start')
    true
  end

  def execute(command)
    stdout, stderr, status = Open3.capture3(command)
    fail "#{command}: #{stderr.chomp}" unless status.success?
    stdout
  end
end
