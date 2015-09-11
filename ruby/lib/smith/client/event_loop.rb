#  File: event_loop.rb
#  Wires up registration independent events and starts/stops event loop
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
          @state.update(Smith::INTERNET_CONNECTED_KEY => true)

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
