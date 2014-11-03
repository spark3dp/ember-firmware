# Class to handle commands that are passed directly to the command pipe
# without any additional action by the client

module Smith
  module Client
    class PrintEngineCommand < Command

      def handle
        acknowledge_command(:received)
        @printer.send_command(@payload.command)
        acknowledge_command(:completed)
      rescue StandardError => e
        acknowledge_command(:failed, "#{e.message} (#{e.class})")
      end

    end
  end
end
