# Class to handle commands that are passed directly to the command pipe
# without any additional action by the client

module Smith
  module Client
    class PrintEngineCommand < Command

      def handle
        acknowledge_command(Command::RECEIVED_ACK)
        Printer.send_command(@payload.command)
        Client.log_info(LogMessages::PRINT_ENGINE_COMMAND_SUCCESS, @payload.command)
        acknowledge_command(Command::COMPLETED_ACK)
      rescue StandardError => e
        Client.log_error(LogMessages::PRINT_ENGINE_COMMAND_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, LogMessages::EXCEPTION_BRIEF, e)
      end

    end
  end
end
