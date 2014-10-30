# Base class for commands

module Smith
  module Client
    class Command

      include URLHelper
      include RequestHelper
      
      def initialize(printer, state, payload)
        @printer, @state, @payload = printer, state, payload
      end

      protected

      def acknowledge_command(state, message = '')
        request = post_request(acknowledge_endpoint,command: @payload.command, command_token: @payload.command_token, state: state, message: message)
        request.callback { Client.log_debug("Successfully acknowledged #{@payload.command.inspect} command (command token #{@payload.command_token.inspect}, state: #{state.inspect}, message: #{message.inspect})") }
      end

    end
  end
end
