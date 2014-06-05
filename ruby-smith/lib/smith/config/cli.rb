require 'thor'

module Smith
  module Config
    class CLI < Thor

      desc 'Load FILE', 'Load configuration from FILE'
      def load(file)
        Network.configure(YAML.load_file(file))
      end

      desc 'set MODE', 'Configure the wireless adapter to operate in managed or adhoc MODE'
      def set(mode)
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
