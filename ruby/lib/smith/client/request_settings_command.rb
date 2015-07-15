# Class to handle command that requests client to upload its settings

module Smith
  module Client

    REQUEST_SETTINGS_COMMAND = 'request_settings'

    class RequestSettingsCommand < Command

      def handle
        acknowledge_command(Command::RECEIVED_ACK)
        # Send acknowledgement with settings in the payload
        acknowledge_command(Command::COMPLETED_ACK, Printer.settings)
      rescue StandardError => e
        Client.log_error(LogMessages::REQUEST_SETTINGS_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, nil, LogMessages::EXCEPTION_BRIEF, e)
      end

      # Send a command acknowledgement that can include the settings.
      # Post request state is the stage of the command acknowledgment
      # If only the state is specified, the message is nil
      # If a string is specified as the third argument, it is formatted as a log message using any additional arguments
      # If something other than a string is specified, it is used directly
      def acknowledge_command(state, settings = nil, *args)
        m = message(*args)
        cp = command_payload(@payload.command, state, m, Printer.get_status)
        if settings
          cp[:data].merge!({ settings: settings })
        end
        request = @http_client.post(acknowledge_endpoint(@payload), cp)
        request.callback { Client.log_debug(LogMessages::ACKNOWLEDGE_COMMAND, @payload.command, @payload.task_id, state, m) }
      end

    end
  end
end
