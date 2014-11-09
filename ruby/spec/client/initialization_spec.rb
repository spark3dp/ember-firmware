require 'client_helper'

module Smith
  describe 'Printer web client when registered', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include ClientPrimaryRegistrationSteps

    let(:initial_printer_state) { { state: HOME_STATE } }

    it 'does not send primary registration commands' do
      set_client_state_async
      write_get_status_command_response_async(initial_printer_state)
      assert_primary_registration_commands_not_sent_when_auth_token_is_known
    end

    context 'when auth token is not valid' do
      it 'clears auth token and printer id and re-attempts primary registration' do
        set_client_state_async(auth_token: 'expired_auth_token', printer_id: 5)

        # Prepare responses to GetStatus command for validation during registration
        write_get_status_command_response_async(state: HOME_STATE)

        # Client attempts registration but gets 403 response
        # Client clears auth token and id
        # Client attempts registration again and succeeds
        assert_primary_registration_code_sent_when_server_initially_reachable
      end
    end

  end
end
