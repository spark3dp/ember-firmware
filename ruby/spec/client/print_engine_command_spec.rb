require 'client_helper'

module Smith
  describe 'Printer web client when print engine command is received', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include PrintEngineCommandSteps

    before { allow_primary_registration }

    it 'forwards command to command pipe' do
      assert_command_forwarded_to_command_pipe(CMD_PAUSE)
    end

  end
end
