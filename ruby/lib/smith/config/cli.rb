require 'thor'

module Smith
  module Config
    class CLI < Thor

      desc 'load FILE', 'Load configuration from FILE'
      def load(file_path)
        Network.configure_from_file(file_path)
      end

      desc 'mode MODE', 'Configure the wireless adapter to operate in managed or ap MODE'
      def mode(mode)
        case mode
        when 'managed'
          WirelessInterface.enable_managed_mode
        when 'ap'
          Network.enable_ap_mode
        else
          puts 'MODE must be managed or ip'
        end
      end

    end
  end
end
