# Base class for commands

module Smith
  module Client
    class Command

      include URLHelper
      
      def initialize(printer, state, payload)
        @printer, @state, @payload = printer, state, payload
      end

      protected

      def acknowledge_command(message)
        request = EM::HttpRequest.new(acknowledge_endpoint).post(
          head: { 'Content-Type' => 'application/json' },
          body: { command: @payload.command, command_token: @payload.command_token, message: message }.to_json
        )

        request.callback { Client.log_debug("Successfully acknowledged #{@payload.command.inspect} command (command token #{@payload.command_token.inspect})") }
        request.errback { Client.log_error("Error making acknowledgement request to server in response to #{@payload.command.inspect} command (command token #{@payload.command_token.inspect})") }
      end

    end
  end
end
