# Class to handle commands that are passed directly to the command pipe
# without any additional action by the client

module Smith
  module Client
    class PrintEngineCommand

      def initialize(printer, payload)
        @printer = printer
        @command = payload[:command]
      end

      def handle
        @printer.send_command(@command)
      end

    end
  end
end
