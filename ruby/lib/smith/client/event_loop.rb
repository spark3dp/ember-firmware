module Smith
  module Client
    class EventLoop

      include URLHelper
      
      def initialize
        @state, @status_pipe = State.load, NamedPipe.status_pipe
        @http_client = HTTPClient.new(@state)
        @registrant = Registrant.new(@state, @http_client)
        @http_client.set_forbidden_callback { @registrant.reregister }
      end

      def start
        Client.log_info(LogMessages::START_EVENT_LOOP)
        EM.run do
          # Start timer to send periodic health checks
          # Registrant#send_health_check only makes requests if the id is known
          EM.add_periodic_timer(Settings.client_health_check_interval) { send_health_check }
      
          # Start watching status pipe for status updates
          # The StatusMonitor only makes requests if the id is known
          status_monitor = EM.watch(@status_pipe.io, StatusMonitor, @state, @http_client)
          status_monitor.notify_readable = true       

          # Attempt to contact the server
          @registrant.attempt_registration 
        end
      end

      def stop
        Client.log_info(LogMessages::STOP_EVENT_LOOP)
        EM.next_tick do
          # Need to allow the event loop/faye client to process the disconnect before stopping
          @registrant.disconnect { EM.stop }
        end
      end

      private

      def send_health_check
        # Only send health check if id is known
        @http_client.post(health_check_endpoint, firmware_version: FIRMWARE_VERSION) if @state.printer_id
      end

    end
  end
end
