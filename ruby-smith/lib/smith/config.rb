require 'smith/config/system'

ENV['WPA_ROAM_PATH']      ||= '/etc/wpa_supplicant'
ENV['WIRELESS_INTERFACE'] ||= 'wlan1'
ENV['WIRED_INTERFACE']    ||= 'eth0'

module Smith
  module Config
    module_function

    def template_path
      File.join(Smith.root, 'lib/smith/config/templates')
    end

    def wpa_roam_file
      File.join(ENV['WPA_ROAM_PATH'], 'wpa-roam.conf')
    end

    def wireless_interface
      ENV['WIRELESS_INTERFACE']
    end

    def wired_interface
      ENV['WIRED_INTERFACE']
    end

    def adhoc_ip
      '192.168.1.1/24'
    end

    def adhoc_ssid
      'beaglebone'
    end

    def available_wireless_networks
      Network.available_wireless_networks
    end
  end
end
