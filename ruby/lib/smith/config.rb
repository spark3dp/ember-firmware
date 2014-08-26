require 'smith'
require 'smith/config/system'

Dir["#{Smith.root}/lib/smith/config/**/*.rb"].each { |f| require(f) }

module Smith
  module Config
    module_function

    def get_template(file_name)
      File.read(File.join(Smith.root, 'lib/smith/config/templates', file_name))
    end

    def wpa_roam_file
      File.join(ENV['WPA_ROAM_DIR'], 'wpa-roam.conf')
    end
    
    def hostapd_config_file
      File.join(ENV['HOSTAPD_CONF_DIR'], 'hostapd.conf')
    end

    def dnsmasq_config_file
      File.join(ENV['DNSMASQ_CONF_DIR'], 'dnsmasq.conf')
    end

    def last_configured_wireless_network_file
      File.join(storage_dir, 'last_wireless_network')
    end

    def ssid_suffix_file
      File.join(storage_dir, 'ssid_suffix')
    end

    def firmware_dir
      ENV['FIRMWARE_DIR']
    end

    def storage_dir
      ENV['STORAGE_DIR']
    end

    def firmware_versions_file
      File.join(ENV['FIRMWARE_DIR'], 'versions')
    end

    def available_wireless_networks
      Network.available_wireless_networks
    end

  end
end
