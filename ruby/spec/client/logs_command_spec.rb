require 'client_helper'
require 'aws-sdk-core'

module Smith
  module Client
    describe 'Printer web client when logs command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include LogsCommandSteps

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
      end

      context 'when upload is possible' do

        it 'uploads log archive and responds to server with location' do |example|
          assert_logs_command_handled_when_logs_command_received
        end

      end

      context 'when server returns status code indicating error' do

        it 'acknowledges error' do
          assert_error_acknowledgement_sent_when_log_command_fails_due_to_http_error_received
        end

      end

      context 'when server is not reachable' do

        it 'acknowledges error' do
          assert_error_acknowledgement_sent_when_log_command_fails_due_to_unreachable_server
        end

      end

    end
  end
end
