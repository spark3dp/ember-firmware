module Smith
  module Client
    module StatusMonitor

      include URLHelper

      def initialize(state)
        @state = state
      end

      def notify_readable
        @io.readlines.each do |status_json|
          # Remove newline
          status_json.sub("\n", '')

          # Ensure that status update is valid JSON
          status = JSON.parse(status_json)

          Client.log_debug("Received status update: #{status.inspect}")

          request = Client.post_request(status_endpoint, status)
          request.callback { Client.log_info("Successfully posted status update containing #{status.inspect} to #{status_endpoint.inspect}") }
        end
      end

    end
  end
end
