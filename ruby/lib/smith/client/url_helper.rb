# Mix in to provide helper methods for building URLs

require 'erb'

module Smith
  module Client
    module URLHelper

      def client_endpoint
        "#{Settings.server_url}/#{Settings.client_endpoint}"
      end

      def registration_endpoint
        "#{Settings.server_url}/#{Settings.server_api_version}/#{Settings.registration_endpoint}"
      end

      def acknowledge_endpoint
        interpolate("#{Settings.server_url}/#{Settings.server_api_version}/#{Settings.acknowledge_endpoint}", @state)
      end

      def registration_channel
        interpolate(Settings.registration_channel, @state)
      end

      def command_channel
        interpolate(Settings.command_channel, @state)
      end

      def interpolate(template, context)
        ERB.new(template).result(context.get_binding)
      end

    end
  end
end
