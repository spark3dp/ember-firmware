require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when status update is written to status pipe', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include ClientStatusUpdateSteps
      include PrintEngineHelper

      before { allow_primary_registration }

      it 'makes request to server with contents of status update' do
        assert_status_update_request_made_when_status_written_to_status_pipe
      end

    end
  end
end
