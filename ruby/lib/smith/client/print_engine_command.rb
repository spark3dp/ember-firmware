# Class to handle commands that are passed directly to the command pipe
# without any additional action by the client

module Smith
  module Client
    class PrintEngineCommand < Command

      def handle
        @printer.send_command(@payload.command)
      ensure
        # Send acknowledgement to server with empty message
        acknowledge_command('')
      end

    end
  end
end
