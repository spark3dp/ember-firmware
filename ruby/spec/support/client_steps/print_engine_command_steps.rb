PrintEngineCommandSteps = RSpec::EM.async_steps do

  def assert_command_acknowledged_and_forwarded_to_command_pipe(expected_command, &callback)
    acknowledgement_notifications = []
    
    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(expected_command)
      d1.succeed
    end
    
    subscription = subscribe_to_test_channel do |payload|
      acknowledgement_notifications.push(payload)
      
      # Wait until 2 ack notification are received
      if acknowledgement_notifications.length == 2
        
        received_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'received' }.first
        completed_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'completed' }.first

        # Verify acknowledgements
        expect(received_ack).not_to be_nil
        expect(received_ack[:request_params][:command]).to eq(expected_command)
        expect(received_ack[:request_params][:command_token]).to eq('123456')
        expect(received_ack[:request_endpoint]).to match(/printers\/539\/acknowledge/)

        expect(completed_ack).not_to be_nil
        expect(completed_ack[:request_params][:command]).to eq(expected_command)
        expect(completed_ack[:request_params][:command_token]).to eq('123456')
        expect(completed_ack[:request_endpoint]).to match(/printers\/539\/acknowledge/)

        subscription.cancel
        d2.succeed
      end
    end

    subscription.callback do
      dummy_server.post('/command', command: expected_command, command_token: '123456')
    end
  end

  def assert_error_acknowledgement_sent_when_print_engine_command_fails(command, &callback)
    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq(command)
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_params][:message]).to match(/Unable to communicate with printer/)
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)

      subscription.cancel
      callback.call
    end

    subscription.callback do
      dummy_server.post('/command', command: command, command_token: '123456')
    end
  end

end
