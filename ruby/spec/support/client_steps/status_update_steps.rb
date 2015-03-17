module Smith
  module Client
    ClientStatusUpdateSteps = RSpec::EM.async_steps do
     
      def assert_status_update_request_made_when_status_written_to_status_pipe(&callback)
        add_http_request_expectation status_endpoint do |request_params|
          expect(request_params.to_json).to eq(test_status_payload.to_json)
          callback.call
        end

        # Simulate status update from smith
        File.write(status_pipe, "#{test_printer_status.to_json}\n")
      end

    end
  end
end
