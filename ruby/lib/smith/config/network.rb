require 'yaml'
require 'erb'

module Smith
  module Config
    module Network
      module_function

      def configure(wireless_network)
        File.write(Config.wpa_roam_file, ERB.new(wireless_network.wpa_roam_template).result(wireless_network.get_binding))
        wireless_network.save_as_last_configured
        Wireless.enable_managed_mode
        Wireless.disconnect if Wired.connected?
      end

      def configure_from_hash(hash)
        configure(WirelessNetwork.new(hash))
      end

      def configure_from_file(file_path)
        configure_from_hash(YAML.load_file(file_path))
      end

      def available_wireless_networks
        IwlistScanParser.parse(Wireless.site_survey)
      end

      def enable_ap_mode
        File.write(Config.hostapd_config_file, ERB.new(Config.get_template('hostapd.conf.erb')).result)
        File.write(Config.dhcpd_config_file, ERB.new(Config.get_template('dhcpd.conf.erb')).result(IPAddress.new(Config.ap_ip).get_binding))
        Wireless.enable_ap_mode
      end

    end
  end
end
