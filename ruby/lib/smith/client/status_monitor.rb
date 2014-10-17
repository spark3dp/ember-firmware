require 'smith/nonblocking_io'

module Smith
  module Client
    module StatusMonitor

      include URLHelper
      include RequestHelper

      def initialize(state)
        @state = state
      end

      def notify_readable
        NonblockingIO.new(@io).readlines_nonblock.each do |status_json|
          # Remove line break character and ensure that status update is valid JSON
          status = JSON.parse(status_json.chomp)

          Client.log_debug("Received status update: #{status.inspect}")

          request = post_request(status_endpoint, status)
          request.callback { Client.log_info("Successfully posted status update containing #{status.inspect} to #{status_endpoint.inspect}") }
        end
      end

    end
  end
end
