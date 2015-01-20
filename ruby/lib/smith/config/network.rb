# Provides high-level functionality for configuring network adapter(s)

require 'yaml'
require 'erb'
require 'fileutils'

module Smith
  module Config
    module Network
      module_function

      def configure(wireless_network)
        File.write(Settings.wpa_roam_file, ERB.new(wireless_network.wpa_roam_template).result(wireless_network.get_binding))
        wireless_network.save_as_last_configured
        WirelessInterface.enable_managed_mode
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
        File.write(Settings.hostapd_config_file, ERB.new(Config.get_template('hostapd.conf.erb')).result(WirelessInterface.ap_mode_config_binding))
        File.write(Settings.dnsmasq_config_file, ERB.new(Config.get_template('dnsmasq.conf.erb')).result(WirelessInterface.ap_mode_config_binding))
        WirelessInterface.enable_ap_mode
      end

      def enter_managed_mode(wireless_network)
        configure(wireless_network)
        Timeout::timeout(Settings.wireless_connection_timeout) do
          loop do
            break if Config::WirelessInterface.connected?
            sleep Settings.wireless_connection_poll_interval
          end
        end
      rescue Timeout::Error
        enable_ap_mode
        File.delete(Settings.wpa_roam_file)
        Printer.show_wireless_connection_failed
      rescue StandardError => e
        $stderr.puts(e.inspect + "\n" + e.backtrace.map{ |l| l.prepend('      ') }.join("\n"))
      else
        Printer.show_wireless_connected
      end

      def init
        enable_ap_mode unless File.file?(Settings.wpa_roam_file)
      end

    end
  end
end
