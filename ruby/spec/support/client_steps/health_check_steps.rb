module Smith
  module Client
    ClientHealthCheckSteps = RSpec::EM.async_steps do

      def assert_periodic_health_checks_made_when_running(&callback)
        # Verify that 2 health check requests are made
        d1 = add_http_request_expectation health_check_endpoint do |request_params|
          expect(request_params[:firmware_version]).to eq(FIRMWARE_VERSION)
        end
        
        d2 = add_http_request_expectation health_check_endpoint do |request_params|
          expect(request_params[:firmware_version]).to eq(FIRMWARE_VERSION)
        end

        when_succeed(d1, d2) { callback.call }

        start_client
      end

      def assert_error_logged_and_health_checks_resume_after_temporary_connection_loss(&callback)

        add_http_request_expectation health_check_endpoint do |request_params|
          expect(request_params[:firmware_version]).to eq(FIRMWARE_VERSION)

          # After getting the first health check, change the server url to simulate unreachable server
          Settings.server_url = 'http://bad.url'
          #bad_health_check_endpoint = health_check_endpoint

          # Next health check request attempts fail
          # Cancel the log subscription as soon as a matching entry is found so that two failed health check requests
          # are guaranteed to have been made
          d1 = add_log_subscription(LogMessages::POST_REQUEST_URL_UNREACHABLE,
                               health_check_endpoint, { firmware_version: FIRMWARE_VERSION }.to_json) { |s| s.cancel }

          d2 = add_log_subscription(LogMessages::POST_REQUEST_URL_UNREACHABLE,
                               health_check_endpoint, { firmware_version: FIRMWARE_VERSION }.to_json) { |s| s.cancel }

          # Two failed health check requests have been made
          when_succeed(d1, d2) do
            # Reset the server url to simulate server becoming reachable again
            Settings.server_url = dummy_server.url

            # Client resumes health checks after server becomes reachable
            add_http_request_expectation health_check_endpoint do |request_params|
              expect(request_params[:firmware_version]).to eq(FIRMWARE_VERSION)

              # Simulate another loss of connectivity
              # Expect two non-debug post request error messages to be logged
              Settings.server_url = 'http://bad.url'
         
              add_log_subscription(LogMessages::HTTP_REQUEST_LOGGING_SUSPENSION, health_check_endpoint) do
                # Check that non-debug post request error message is logged only when request fails for the first time
                # There are two such scenarios in this test
                expect(grep_log(LogMessages::HTTP_REQUEST_LOGGING_SUSPENSION, health_check_endpoint).length).to eq(2)

                callback.call
              end

            end
          end
        end

        start_client
      end

    end
  end
end
