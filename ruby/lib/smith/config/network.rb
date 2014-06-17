require 'yaml'
require 'erb'

module Smith
  module Config
    module Network
      module_function

      def configure(wireless_network)
        File.write(Config.wpa_roam_file, ERB.new(wireless_network.wpa_roam_template).result(wireless_network.get_binding))
        wireless_network.save_as_last_configured
        WirelessInterface.enable_managed_mode
        WirelessInterface.disconnect if WiredInterface.connected?
      end

      def configure_from_hash(hash)
        configure(WirelessNetwork.new(hash))
      end

      def configure_from_file(file_path)
        configure_from_hash(YAML.load_file(file_path))
      end

      def available_wireless_networks
        IwlistScanParser.parse(WirelessInterface.site_survey)
      end

      def enable_ap_mode
        File.write(Config.hostapd_config_file, ERB.new(Config.get_template('hostapd.conf.erb')).result(WirelessInterface.ap_mode_config_binding))
        File.write(Config.dnsmasq_config_file, ERB.new(Config.get_template('dnsmasq.conf.erb')).result(WirelessInterface.ap_mode_config_binding))
        WirelessInterface.enable_ap_mode
      end

    end
  end
end
