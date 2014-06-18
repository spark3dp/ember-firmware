require 'smith/config/system'

# Config options can be set through environment variables
# These are the defaults if not set
ENV['WPA_ROAM_PATH']      ||= '/etc/wpa_supplicant'
ENV['HOSTAPD_CONF_PATH']  ||= '/etc/hostapd'
ENV['DNSMASQ_CONF_PATH']  ||= '/etc'
ENV['STORAGE_PATH']       ||= '/var/local'
ENV['WIRELESS_INTERFACE'] ||= 'wlan1'
ENV['WIRED_INTERFACE']    ||= 'eth0'
ENV['AP_SSID']            ||= 'beaglebone'
ENV['AP_IP']              ||= '192.168.1.1/24'

module Smith
  module Config
    module_function

    def get_template(file_name)
      File.read(File.join(Smith.root, 'lib/smith/config/templates', file_name))
    end

    def wpa_roam_file
      File.join(ENV['WPA_ROAM_PATH'], 'wpa-roam.conf')
    end
    
    def hostapd_config_file
      File.join(ENV['HOSTAPD_CONF_PATH'], 'hostapd.conf')
    end

    def dnsmasq_config_file
      File.join(ENV['DNSMASQ_CONF_PATH'], 'dnsmasq.conf')
    end

    def last_configured_wireless_network_file
      File.join(ENV['STORAGE_PATH'], 'last_wireless_network')
    end

    def available_wireless_networks
      Network.available_wireless_networks
    end

  end
end
