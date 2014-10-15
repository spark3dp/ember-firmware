module Smith
  module Client
    class EventLoop

      def initialize
        @registrant = Registrant.new
      end

      def start
        Client.log_info('Starting event loop')
        EM.run do

          # Setup signal handling
          Signal.trap('INT') { stop }
          Signal.trap('TERM') { stop }

          # Attempt to contact the server
          @registrant.attempt_registration 

        end
      rescue
        # In case of an exception, attempt to disconnect and re-raise the exception
        @registrant.disconnect
        raise
      end

      def stop
        Client.log_info('Stopping event loop')
        EM.next_tick do

          @registrant.disconnect

          # Need to allow the event loop to process the disconnect before stopping
          EM.next_tick { EM.stop }

        end
      end

    end
  end
end
