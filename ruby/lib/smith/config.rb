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
