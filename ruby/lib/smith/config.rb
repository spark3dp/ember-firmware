require 'smith/config/system'

# Config options can be set through environment variables
# These are the defaults if not set
ENV['WPA_ROAM_PATH']      ||= '/etc/wpa_supplicant'
ENV['WIRELESS_INTERFACE'] ||= 'wlan1'
ENV['WIRED_INTERFACE']    ||= 'eth0'
ENV['ADHOC_SSID']	  ||= 'beaglebone'
ENV['ADHOC_IP']		  ||= '192.168.1.1/24'
ENV['STORAGE_PATH']	  ||= '/var'

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
      ENV['ADHOC_IP']
    end

    def adhoc_ssid
      ENV['ADHOC_SSID']
    end

    def last_configured_wireless_network_file
      File.join(ENV['STORAGE_PATH'], 'smith_last_wireless_network')
    end

    def available_wireless_networks
      Network.available_wireless_networks
    end
  end
end
