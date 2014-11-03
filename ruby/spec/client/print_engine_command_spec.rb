require 'client_helper'

module Smith
  describe 'Printer web client when print engine command is received', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include PrintEngineCommandSteps

    before { allow_primary_registration }

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
