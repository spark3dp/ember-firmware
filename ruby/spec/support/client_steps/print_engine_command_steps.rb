module Smith
  PrintEngineCommandSteps = RSpec::EM.async_steps do

    def assert_command_acknowledged_and_forwarded_to_command_pipe(command, &callback)
      d1 = add_command_pipe_expectation do |command|
        expect(command).to eq(command)
      end

      d2 = add_http_request_expectation acknowledge_endpoint do |request_params|
        expect(request_params[:state]).to eq('received')
        expect(request_params[:command]).to eq(command)
        expect(request_params[:command_token]).to eq('123456')
      end

      d3 = add_http_request_expectation acknowledge_endpoint do |request_params|
        expect(request_params[:state]).to eq('completed')
        expect(request_params[:command]).to eq(command)
        expect(request_params[:command_token]).to eq('123456')
      end

      when_succeed(d1, d2, d3) { callback.call }

      dummy_server.post('/command', command: command, command_token: '123456')
    end

    def assert_error_acknowledgement_sent_when_print_engine_command_fails(command, &callback)
      d1 = add_http_request_expectation acknowledge_endpoint do |request_params|
        expect(request_params[:state]).to eq('received')
        expect(request_params[:command]).to eq(command)
        expect(request_params[:command_token]).to eq('123456')
      end

      d2 = add_http_request_expectation acknowledge_endpoint do |request_params|
        expect(request_params[:state]).to eq('failed')
        expect(request_params[:command]).to eq(command)
        expect(request_params[:command_token]).to eq('123456')
        expect(request_params[:message]).to match(/Unable to communicate with printer/)
      end

      when_succeed(d1, d2) { callback.call }

      dummy_server.post('/command', command: command, command_token: '123456')
    end

  end
end
