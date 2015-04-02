require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when print engine command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include PrintEngineCommandSteps

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
      end

      context 'when incoming message is a Hash' do
        it 'forwards command to command pipe and sends command acknowledgements' do
          assert_command_acknowledged_and_forwarded_to_command_pipe_when_message_is_a_hash
        end
      end

      context 'when no errors are raised during command handling' do
        it 'forwards command to command pipe and sends command acknowledgements' do
          assert_command_acknowledged_and_forwarded_to_command_pipe(CMD_PAUSE)
        end
      end

      context 'when an error is raised during command handling' do
        it 'acknowledges error' do
          # Close the command pipe to simulate error condition
          close_command_pipe_async
          assert_error_acknowledgement_sent_when_print_engine_command_fails(CMD_PAUSE)
        end
      end

    end
  end
end
