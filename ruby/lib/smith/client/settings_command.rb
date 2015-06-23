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

    end
  end
end
