ClientHealthCheckSteps = RSpec::EM.async_steps do

  def assert_periodic_health_checks_made_when_running(&callback)
    health_check_count = 0

    subscription = subscribe_to_test_channel do |payload|
      if dummy_server.url + payload[:request_endpoint] == health_check_endpoint
        health_check_count += 1
       
        expect(payload[:request_params]).to eq(firmware_version: Smith::FIRMWARE_VERSION)
      
        # Verify that 2 health check requests are made
        if health_check_count == 2
          subscription.cancel
          callback.call
        end
      end
    end

    subscription.callback do
      start_client
    end
  end

  def assert_error_logged_and_health_checks_resume_after_temporary_connection_loss(&callback)
    subscription = subscribe_to_test_channel do |payload|
      load_state
   
      if dummy_server.url + payload[:request_endpoint] == health_check_endpoint 
        subscription.cancel
        expect(payload[:request_params]).to eq(firmware_version: Smith::FIRMWARE_VERSION)
      
        # After getting the first health check, change the server url to simulate unreachable server
        Smith::Settings.server_url = 'http://bad.url'
        
        expected_entry = Smith::Client::LogMessage.format(
          Smith::Client::LogMessages::POST_REQUEST_URL_UNREACHABLE,
          health_check_endpoint,
          { firmware_version: Smith::FIRMWARE_VERSION }.to_json
        )

        # Next health check request attempt fails
        add_log_subscription(expected_entry) do
          # Reset the server url to simulate server becoming reachable again
          Smith::Settings.server_url = dummy_server.url

          # Client resumes health checks after server becomes reachable
          subscription = subscribe_to_test_channel do |payload|
            subscription.cancel
            
            if dummy_server.url + payload[:request_endpoint] == health_check_endpoint 
              expect(payload[:request_params]).to eq(firmware_version: Smith::FIRMWARE_VERSION)
            
              callback.call
            end
          end

        end
      end
    end

    subscription.callback do
      start_client
    end
  end

end
