# Steps that are shared between different contexts
module Smith
  module Client
    ClientSteps = RSpec::EM.async_steps do

      # Start the client and wait unitl primary registration is complete
      # Does not make any expectations on registration commands to avoid over testing
      def allow_primary_registration(&callback)
        set_printer_status(state: HOME_STATE)

        d1 = add_command_pipe_expectation do |actual_command|
          expect(actual_command).to eq(CMD_REGISTRATION_CODE)
          dummy_server.post_registration(registration_code: dummy_server.registration_code)
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
          expect(request_params.to_json).to eq(test_status_payload.to_json)
        end

        d2 = add_log_subscription(LogMessages::SUBSCRIPTION_SUCCESS, command_channel) do
          # Now that client is listening for commands, send a command to ensure that nothing was written to command pipe on startup
          dummy_server.post_command(command: CMD_PAUSE)
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
end
