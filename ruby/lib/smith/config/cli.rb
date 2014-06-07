require 'thor'

module Smith
  module Config
    class CLI < Thor

      desc 'load FILE', 'Load configuration from FILE'
      def load(file_path)
        Network.configure_from_file(file_path)
      end

      desc 'mode MODE', 'Configure the wireless adapter to operate in managed or adhoc MODE'
      def mode(mode)
        case mode
        when 'managed'
          Wireless.enable_managed_mode
        when 'adhoc'
          Wireless.enable_adhoc_mode
        else
          puts 'MODE must be managed or adhoc'
        end
      end

    end
  end
end
