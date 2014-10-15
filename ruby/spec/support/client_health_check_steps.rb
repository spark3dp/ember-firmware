ClientHealthCheckSteps = RSpec::EM.async_steps do

  def assert_periodic_health_checks_made_when_running(&callback)
    health_check_count = 0

    subscription = subscribe_to_test_channel do |payload|
      health_check_count += 1
      
      expect(payload[:request_endpoint]).to match(/printers\/539\/health_check/)
      expect(payload[:request_params]).to eq(firmware_version: Smith::FIRMWARE_VERSION)
    
      # Verify that 2 health check requests are made
      if health_check_count == 2
        subscription.cancel
        callback.call
      end
    end

    subscription.callback do
      update_state_printer_registered
      start_client
    end
  end

  def assert_error_logged_and_health_checks_resume_after_temporary_connection_loss(&callback)
    subscription = subscribe_to_test_channel do |payload|
      expect(payload[:request_endpoint]).to match(/printers\/539\/health_check/)
      expect(payload[:request_params]).to eq(firmware_version: Smith::FIRMWARE_VERSION)
    
      # After getting the first health check, change the server url to simulate unreachable server
      Smith::Settings.server_url = 'http://bad.url'
      subscription.cancel
    end

    # Next health check request attempt fails
    add_error_log_expectation do |line|
      expect(line).to match(/Attempt to post health check failed/)
    
      # Reset the server url to simulate server becoming reachable again
      Smith::Settings.server_url = dummy_server.url

      # Client resumes health checks after server becomes reachable
      subscription = subscribe_to_test_channel do |payload|
        expect(payload[:request_endpoint]).to match(/printers\/539\/health_check/)
        expect(payload[:request_params]).to eq(firmware_version: Smith::FIRMWARE_VERSION)
      
        subscription.cancel
        callback.call
      end

    end

    subscription.callback do
      update_state_printer_registered
      start_client
    end
  end

end
