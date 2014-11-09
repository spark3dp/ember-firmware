require 'client_helper'

module Smith
  describe 'Printer web client when started when not registered', :client do
    # See support/client_context.rb for setup/teardown
    include ClientPrimaryRegistrationSteps

    context 'when server is reachable and printer is in home state' do

      it 'responds to registration code entry notification' do
        # Prepare response to GetStatus command send during validation
        write_get_status_command_response_async(state: HOME_STATE)
        
        # Client contacts server and receives registration code
        # Client subscribes to registration notification channel
        # Client commands smith to display registration code
        assert_primary_registration_code_sent_when_server_initially_reachable
        
        # User enters registration code into web portal and client receives notification from server
        # Client is notified of code entry by server and client commands smith to display registration success screen
        assert_primary_registration_succeeded_sent_when_notified_of_registration_code_entry

        # Authentication token and printer id are persisted 
        assert_identity_persisted
      end

    end

    context 'when server is not reachable initially but becomes reachable after retry interval' do

      it 'reattempts to reach server and responds to registration code entry notification' do
        write_get_status_command_response_async(state: HOME_STATE)
        write_get_status_command_response_async(state: HOME_STATE)
        
        # Simulate unreachable server by setting server url to invalid value
        set_settings_async(server_url: 'http://bad.url')
       
        # Need to to check the log so we can hook in to event loop to change server URL
        assert_warn_log_entry_written_when_server_is_not_initially_reachable
        
        # Simulate server becoming reachable
        set_settings_async(server_url: dummy_server.url)

        # Client does nothing during retry interval
        # After retry interval client can reach server and receives registration code
        # Client subscribes to registration notification channel
        # Client commands smith to display registration code
        assert_primary_registration_code_sent
        
        # Authentication token and printer id are persisted 
        assert_identity_persisted
      end

    end

    context 'when printer is not in home state initially but is in home state after retry interval' do
      it 'reattempts to reach server and responds to registration code entry notification' do
        # Prepare responses to GetStatus command
        write_get_status_command_response_async(state: PRINTING_STATE)
        write_get_status_command_response_async(state: HOME_STATE)

        assert_warn_log_entry_written_when_printer_not_initially_in_home_state

        # Client does nothing during retry interval
        # After retry interval printer is in home state
        # Client subscribes to registration notification channel
        # Client commands smith to display registration code
        assert_primary_registration_code_sent
       
        # Authentication token and printer id are persisted 
        assert_identity_persisted
      end

    end

  end

end
