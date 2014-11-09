ClientPrimaryRegistrationSteps = RSpec::EM.async_steps do
  def assert_primary_registration_code_sent_when_server_initially_reachable(&callback)
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_REGISTRATION_CODE)
      expect(registration_file_contents).to eq(Smith::REGISTRATION_CODE_KEY => '4321', Smith::REGISTRATION_URL_KEY => 'registration url')
      callback.call
    end

    start_client
  end

  def assert_primary_registration_succeeded_sent_when_notified_of_registration_code_entry(&callback)
    # Expect both a health check to be posted to server and the registration complete command to be sent to command pipe

    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_REGISTERED)
      d1.succeed
    end
    
    subscription = subscribe_to_test_channel do |payload|
      load_state
      if dummy_server.url + payload[:request_endpoint] == health_check_endpoint
        subscription.cancel
        d2.succeed
      end
    end

    subscription.callback do
      # Simulate user entering registration code into portal
      dummy_server.post('/v1/user/printers', registration_code: '4321')
    end
  end

  def assert_warn_log_entry_written_when_server_is_not_initially_reachable(&callback)
    add_warn_log_expectation do |line|
      expect(line).to match(/#{
        Smith::Client::LogMessage.format(
          Smith::Client::LogMessages::RETRY_REGISTRATION_AFTER_REQUEST_FAILED,
          Smith::Settings.server_url,
          Smith::Settings.client_retry_interval
      )}/)
      callback.call
    end
    
    start_client
  end

  def assert_primary_registration_code_sent(&callback)
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_REGISTRATION_CODE)
      expect(registration_file_contents).to eq(Smith::REGISTRATION_CODE_KEY => '4321', Smith::REGISTRATION_URL_KEY => 'registration url')
      callback.call
    end
  end

  def assert_warn_log_entry_written_when_printer_not_initially_in_home_state(&callback)
    add_warn_log_expectation do |line|
      expect(line).to match(/#{
        Smith::Client::LogMessage.format(
          Smith::Client::LogMessages::RETRY_REGISTRATION_AFTER_ERROR,
          '.*?',
          Smith::Settings.client_retry_interval
      )}/)
      callback.call
    end

    start_client
  end

  def assert_identity_persisted(&callback)
    state = Smith::State.load
    expect(state.printer_id).to eq(539)
    expect(state.auth_token).to eq('authtoken')
    callback.call
  end

end
