require 'yaml'
require 'erb'

module Smith
  module Config
    module Network
      module_function

      def configure(options)
        config = ConfigOptions.new(options)
        File.write(Config.wpa_roam_file, ERB.new(config.get_template).result(config.get_binding))
        Wireless.enable_managed_mode
        Wireless.disconnect if Wired.connected?
      end

      def available_wireless_networks
        IwlistScanParser.parse(Wireless.site_survey)
      end
    end
  end
end
