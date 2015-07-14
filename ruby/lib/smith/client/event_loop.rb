module Smith
  module Client
    class EventLoop

      include URLHelper
      include PayloadHelper
      
      def initialize(state)
        @state, @status_pipe = state, NamedPipe.status_pipe
        @http_client = HTTPClient.new(@state)
        @registrant = Registrant.new(@state, @http_client)
        @http_client.set_forbidden_callback { @registrant.reregister }
      end

      def start
        Client.log_info(LogMessages::START_EVENT_LOOP)
        EM.run do
          # Start timer to send periodic status updates
          # Registrant#send_periodic_status only makes requests if the id is known
          EM.add_periodic_timer(Settings.client_periodic_status_interval) { send_periodic_status }
      
          # Start watching status pipe for status updates
          # The StatusMonitor only makes requests if the id is known
          status_monitor = EM.watch(@status_pipe.io, StatusMonitor, @state, @http_client)
          status_monitor.notify_readable = true       

          # initialize the internet_connected flag
          # flag is updated to false if attempt_registration fails
          @state.update(internet_connected: true)

          # Attempt to contact the server
          @registrant.attempt_registration 
        end
      end

      def stop
        Client.log_info(LogMessages::STOP_EVENT_LOOP)
        # Need to allow the event loop/faye client to process the disconnect before stopping
        @registrant.disconnect { EM.stop }
      end

      private

      def send_periodic_status
        # Only send periodic status if printer id is known
        @http_client.post(status_endpoint, status_payload(Printer.get_status)) if @state.printer_id
      end

    end
  end
end
