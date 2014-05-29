require 'open4'

module Configurator
  module Wireless
    module_function

    def enable_managed_mode(interface)
      execute("wpa_action #{interface} stop")
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
      stderr_str = nil
      puts "[INFO] Executing #{command}:"
      status = Open4.popen4(command) do |pid, stdin, stdout, stderr|
        stdout_str = stdout.read
        stderr_str = stderr.read
        puts stdout_str unless stdout_str.empty?
      end
      print "\n"
      fail "#{command}: #{stderr_str}" unless status.success?
    end
  end
end
