require 'smith/config/system'

# Config options can be set through environment variables
# These are the defaults if not set
ENV['WPA_ROAM_PATH']      ||= '/etc/wpa_supplicant'
ENV['HOSTAPD_CONF_PATH']  ||= '/etc/hostapd'
ENV['DHCPD_CONF_PATH']    ||= '/etc/dhcp'
ENV['WIRELESS_INTERFACE'] ||= 'wlan1'
ENV['WIRED_INTERFACE']    ||= 'eth0'
ENV['AP_SSID']            ||= 'beaglebone'
ENV['AP_IP']              ||= '192.168.1.1/24'
ENV['STORAGE_PATH']       ||= '/var'

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

    def dhcpd_config_file
      File.join(ENV['DHCPD_CONF_PATH'], 'dhcpd.conf')
    end

    def last_configured_wireless_network_file
      File.join(ENV['STORAGE_PATH'], 'smith_last_wireless_network')
    end

    def wireless_interface
      ENV['WIRELESS_INTERFACE']
    end

    def wired_interface
      ENV['WIRED_INTERFACE']
    end

    def ap_ip
      ENV['AP_IP']
    end

    def ap_ssid
      ENV['AP_SSID']
    end

    def available_wireless_networks
      Network.available_wireless_networks
    end

  end
end
