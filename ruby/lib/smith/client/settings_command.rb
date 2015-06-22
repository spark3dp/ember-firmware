# Class to handle settings command
# Sends commands necessary to apply settings

module Smith
  module Client

    SETTINGS_COMMAND = 'settings'

    class SettingsCommand < Command

      def handle
        # Send acknowledgement to server with empty message
        acknowledge_command(Command::RECEIVED_ACK)

        # apply the specified settings
        apply_settings
      end

      private

      def apply_settings
        Printer.write_settings_file(@payload.settings)
        Printer.apply_settings_file
        acknowledge_command(Command::COMPLETED_ACK)
      rescue StandardError => e
        Client.log_error(LogMessages::SETTINGS_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, LogMessages::EXCEPTION_BRIEF, e)
      end

      # Send an command acknowledgement that includes the job_id.
      # Post request state is the stage of the command acknowledgment
      # If only the state is specified, the message is nil
      # If a string is specified as the third argument, it is formatted as a log message using any additional arguments
      # If something other than a string is specified, it is used directly
      def acknowledge_command(state, *args)
        m = message(*args)
        request = @http_client.post(acknowledge_endpoint(@payload), command_payload(@payload.command, state, m, Printer.get_status, @payload.job_id))
        request.callback { Client.log_debug(LogMessages::ACKNOWLEDGE_COMMAND, @payload.command, @payload.task_id, state, m) }
      end
    end
  end
end
