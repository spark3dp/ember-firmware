PrintEngineCommandSteps = RSpec::EM.async_steps do

  def assert_command_forwarded_to_command_pipe(expected_command, &callback)
    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(expected_command)
      d1.succeed
    end
    
    # Subscribe to the test channel to receive notification of client posting command acknowledgement
    subscription = subscribe_to_test_channel do |payload|
      expect(payload[:request_params][:state]).to eq('received')
      expect(payload[:request_params][:command]).to eq(expected_command)
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)
      subscription.cancel
      d2.succeed
    end

    subscription.callback do
      dummy_server.post('/command', command: expected_command, command_token: '123456')
    end
  end

end
