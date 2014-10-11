require 'client_helper'

module Smith
  describe 'Printer web client when client has already been registered', :client do
    include ClientSteps

    it 'does not send primary registration commands' do
      assert_primary_registration_commands_not_sent_when_auth_token_is_known
    end

  end
end
