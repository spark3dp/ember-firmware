# Class to handle commands that are passed directly to the command pipe
# without any additional action by the client

module Smith
  module Client
    class PrintEngineCommand

      include URLHelper

      def initialize(printer, state, payload)
        @printer, @state = printer, state
        @command, @command_token = payload.values_at :command, :command_token
      end

      def handle
        @printer.send_command(@command)
      ensure
        # Send acknowledgement to server
        request = EM::HttpRequest.new(acknowledge_endpoint(@state)).post(
          head: { 'Content-Type' => 'application/json' },
          body: { command: @command, command_token: @command_token, message: '' }.to_json
        )

        request.callback { Client.log_debug("Successfully acknowledged #{@command.inspect} command (command token #{@command_token.inspect})") }
        request.errback { Client.log_error("Error making acknowledgement request to server in response to #{@command.inspect} command (command token #{@command_token.inspect})") }
      end

    end
  end
end
