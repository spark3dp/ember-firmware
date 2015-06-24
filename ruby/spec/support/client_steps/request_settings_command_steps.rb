require 'fileutils'

module Smith
  module Client
    RequestSettingsCommandSteps = RSpec::EM.async_steps do

      def assert_request_settings_command_handled_when_request_settings_command_received(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command

        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:data][:settings][:ProjectorLEDCurrent]).to eq(200)
          expect(request_params[:data][:settings][:InspectionHeightMicrons]).to eq(65000)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: REQUEST_SETTINGS_COMMAND,
          task_id: test_task_id,
        )
      end

      def assert_error_acknowledgement_sent_when_request_settings_command_fails(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
   #       expect(request_params[:data][:message]).to match_log_message(
   #               LogMessages::EXCEPTION_BRIEF,
   #               Printer::CommunicationError.new(''))
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: REQUEST_SETTINGS_COMMAND,
          task_id: test_task_id,
        )
      end

    end
  end
end
