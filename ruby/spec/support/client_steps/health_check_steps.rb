module Smith
  module Client
    ClientHealthCheckSteps = RSpec::EM.async_steps do

      def assert_periodic_health_checks_made_when_running(&callback)
        # Verify that 2 health check requests are made
        d1 = add_http_request_expectation status_endpoint do |request_params|
          expect(request_params[:state]).to eq(PRINTING_STATE)
        end
        
        d2 = add_http_request_expectation status_endpoint do |request_params|
          expect(request_params[:state]).to eq(PRINTING_STATE)
        end

        when_succeed(d1, d2) { callback.call }

        start_client
      end

      def assert_error_logged_and_health_checks_resume_after_temporary_connection_loss(&callback)

        good_status_endpoint = status_endpoint
       
        # Wait for a health check to complete successfully 
        add_log_subscription(LogMessages::POST_REQUEST_SUCCESS,
                             good_status_endpoint, { state: PRINTING_STATE, ui_sub_state: nil }.to_json) do |subscription|

          subscription.cancel

          # After getting the first health check, change the server url to simulate unreachable server
          Settings.server_url = 'http://bad.url'
          bad_status_endpoint = status_endpoint

          # Next health check request attempts fail
          # Cancel the log subscription as soon as a matching entry is found so that two failed health check requests
          # are guaranteed to have been made
          d1 = add_log_subscription(LogMessages::POST_REQUEST_URL_UNREACHABLE,
                               bad_status_endpoint, { state: PRINTING_STATE, ui_sub_state: nil }.to_json) { |s| s.cancel }

          d2 = add_log_subscription(LogMessages::POST_REQUEST_URL_UNREACHABLE,
                               bad_status_endpoint, { state: PRINTING_STATE, ui_sub_state: nil }.to_json) { |s| s.cancel }

          # Two failed health check requests have been made
          when_succeed(d1, d2) do
       
            # Wait for health checks to start succeeding 
            d3 = add_http_request_expectation good_status_endpoint

            # Wait for log entry indicating that health checks are succeeding
            d4 = add_log_subscription(LogMessages::HTTP_REQUEST_LOGGING_RESUMPTION)
           
            when_succeed(d3, d4) do
              # Check that non-debug post request error message is logged only when request fails for the first time
              # If logging suspension was not working in the HTTPClient, then there would be two error messages since
              # the above subscriptions to POST_REQUEST_URL_UNREACHABLE ensure that two failed requests were made
              expect(grep_log(LogMessages::HTTP_REQUEST_LOGGING_SUSPENSION, bad_status_endpoint).length).to eq(1)
              callback.call
            end

            # Reset the server url to simulate server becoming reachable again
            Settings.server_url = dummy_server.url

          end
        end

        start_client
      end

    end
  end
end
