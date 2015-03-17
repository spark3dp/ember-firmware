require 'smith/nonblocking_io'

module Smith
  module Client
    module StatusMonitor

      include URLHelper
      include PayloadHelper

      def initialize(state, http_client)
        @state, @http_client = state, http_client
      end

      def notify_readable
        NonblockingIO.new(@io).readlines_nonblock.each do |status_json|
          # Remove line break character and ensure that status update is valid JSON
          status = JSON.parse(status_json.chomp)

          Client.log_debug(LogMessages::RECEIVE_STATUS_UPDATE, status)

          # Only make a request is the id is known
          @http_client.post(status_endpoint, status_payload(status)) if @state.printer_id
        end
      end

    end
  end
end
