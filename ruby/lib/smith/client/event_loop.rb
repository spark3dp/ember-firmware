module Smith
  module Client
    class EventLoop

      def initialize(state, retry_interval)
        @registrant = Registrant.new(state, retry_interval)
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
        # In case of an exception, attempt to disconnect the client and re-raise the exception
        @registrant.disconnect_client
        raise
      end

      def stop
        Client.log_info('Stopping event loop')
        EM.next_tick do

          @registrant.disconnect_client

          # Need to allow the event loop to process the disconnect before stopping
          EM.next_tick { EM.stop }

        end
      end

    end
  end
end
