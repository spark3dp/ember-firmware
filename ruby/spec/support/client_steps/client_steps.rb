# Steps that are shared between different contexts
ClientSteps = RSpec::EM.async_steps do

  # Start the client and wait unitl primary registration is complete
  # Does not make any expectations on registration commands to avoid over testing
  def allow_primary_registration(&callback)
    write_get_status_command_response(state: Smith::HOME_STATE)
    expect_get_status_command

    add_command_pipe_allowance do
      # this callback gets called when the CMD_REGISTRATION_CODE command is sent
      # the callback "callback" gets called when CMD_REGISTERED is sent
      dummy_server.post('/v1/user/printers', registration_code: '4321')
    end
    
    add_command_pipe_allowance(&callback)

    start_client
  end

  def assert_primary_registration_commands_not_sent_when_auth_token_is_known(&callback)
    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    # Check that the client sends out a status update after it finishes initialization
    subscription = subscribe_to_test_channel do |payload|
      if dummy_server.url + payload[:request_endpoint] == status_endpoint
        # Encode/decode printer status to convert keys to strings
        expect(JSON.parse(payload[:request_params].to_json)).to eq(printer_status(initial_printer_state))
        subscription.cancel
        d1.succeed
      end
    end

    add_log_subscription(Smith::Client::LogMessage.format(Smith::Client::LogMessages::SUBSCRIPTION_SUCCESS, command_channel)) do
      # Expect first command client sends to be incoming pause command from server sent below
      add_command_pipe_expectation do |command|
        expect(command).to eq(Smith::CMD_PAUSE)
        d2.succeed
      end
      
      # Now that client is listening for commands, send a command to ensure that nothing was written to command pipe on startup
      dummy_server.post('/command', command: Smith::CMD_PAUSE)
    end

    # Simulate startup of client when auth token is known
    # Server does not send back registration code if client sends auth token on initial registration request
    subscription.callback do
      start_client
    end
  end

end
