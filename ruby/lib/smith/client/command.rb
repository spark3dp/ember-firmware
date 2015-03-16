# Base class for commands

module Smith
  module Client
    class Command

      COMPLETED_ACK = 'completed'
      RECEIVED_ACK = 'received'
      FAILED_ACK = 'failed'

      include URLHelper
      
      def initialize(state, http_client, payload)
        @state, @http_client, @payload = state, http_client, payload
      end

      private 

      # Send an command acknowledgement post request state is the stage of the command acknowledgment
      # If only the state is specified, the message is nil
      # If a string is specified as the second argument, it is formatted as a log message using any additional arguments
      # If something other than a string is specified, it is used directly
      def acknowledge_command(state, *args)
        m = message(*args)
        request = @http_client.post(acknowledge_endpoint(@payload), command: @payload.command, state: state, message: m)
        request.callback { Client.log_debug(LogMessages::ACKNOWLEDGE_COMMAND, @payload.command, @payload.task_id, state, m) }
      end

      # Get the message portion of the acknowledgment according to the specified arguments
      def message(*args)
        return                          if args.empty?
        return LogMessage.format(*args) if args.first.is_a?(String)
        return args.first
      end

    end
  end
end
