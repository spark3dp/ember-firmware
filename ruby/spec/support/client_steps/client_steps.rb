# Steps that are shared between different contexts
module Smith
  ClientSteps = RSpec::EM.async_steps do

    # Start the client and wait unitl primary registration is complete
    # Does not make any expectations on registration commands to avoid over testing
    def allow_primary_registration(&callback)
      write_get_status_command_response(state: HOME_STATE)
      allow_get_status_command

      d1 = add_command_pipe_expectation do |actual_command|
        expect(actual_command).to eq(CMD_REGISTRATION_CODE)
        dummy_server.post('/v1/user/printers', registration_code: '4321')
      end

      d2 = add_command_pipe_expectation do |actual_command|
        expect(actual_command).to eq(CMD_REGISTERED)
      end

      when_succeed(d1, d2) { callback.call }

      start_client
    end

    def assert_primary_registration_commands_not_sent_when_auth_token_is_known(&callback)
      
      # Check that the client sends out a status update after it finishes initialization
      d1 = add_http_request_expectation status_endpoint do |request_params|
        # Encode/decode printer status to convert keys to strings
        expect(JSON.parse(request_params.to_json)).to eq(printer_status(initial_printer_state))
      end

      d2 = add_log_subscription(Client::LogMessages::SUBSCRIPTION_SUCCESS, command_channel) do
        # Now that client is listening for commands, send a command to ensure that nothing was written to command pipe on startup
        dummy_server.post('/command', command: CMD_PAUSE)
      end

      # Expect first command client sends to be incoming pause command from server sent below
      d3 = add_command_pipe_expectation do |command|
        expect(command).to eq(CMD_PAUSE)
      end

      when_succeed(d1, d2, d3) { callback.call }

      # Simulate startup of client when auth token is known
      # Server does not send back registration code if client sends auth token on initial registration request
      start_client
    end

  end
end
