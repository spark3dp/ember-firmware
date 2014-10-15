require 'client_helper'

module Smith
  describe 'Printer web client when running', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include ClientHealthCheckSteps

    # Set interval to small value so tests don't take unnecessarily long
    # Keep interval greater than zero to avoid making many requests
    let(:health_check_interval) { 0.05 }

    it 'makes periodic health check requests to server' do
      # assert that client makes two health check requests
      assert_periodic_health_checks_made_when_running
    end

    context 'when server becomes temporarily unreachable' do
      it 'continues making health check requests after re-establishing contact with server' do
        assert_error_logged_and_health_checks_resume_after_temporary_connection_loss
      end
    end

  end
end
