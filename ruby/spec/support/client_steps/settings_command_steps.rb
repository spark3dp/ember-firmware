require 'fileutils'

module Smith
  module Client
    SettingsCommandSteps = RSpec::EM.async_steps do

      def assert_settings_command_handled_when_settings_command_received(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command

        end

        d2 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_APPLY_SETTINGS)
          expect(test_settings_file_contents).to eq(test_settings)
        end

        d3 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        when_succeed(d1, d2, d3) { callback.call }

        dummy_server.post_command(
          command: SETTINGS_COMMAND,
          task_id: test_task_id,
          settings: test_settings,
        )
      end

      def assert_error_acknowledgement_sent_when_settings_command_fails(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
          expect(request_params[:data][:message]).to match_log_message(
                  LogMessages::EXCEPTION_BRIEF,
                  Printer::CommunicationError.new(''))
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: SETTINGS_COMMAND,
          task_id: test_task_id,
          settings: test_settings
        )
      end

    end
  end
end
