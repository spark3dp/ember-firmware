require 'client_helper'

module Smith
  describe 'Printer web client when started', :client do
    include PrintEngineHelperAsync
    include LogHelperAsync
    include ClientSteps

    let(:retry_interval) { 0 }
    
    before do
      create_command_pipe_async
      create_command_response_pipe_async
      open_command_response_pipe_async

      # Set argument to true to print client log output to stdout
      watch_log_async(false)
      
      # Make printer reachable by default
      set_settings_async(server_url: dummy_server.url)
    end

    after do
      stop_client_async
      close_command_pipe_async
      close_command_response_pipe_async
    end

    context 'when command pipe is open for reading' do
      before { watch_command_pipe_async }

      context 'when printer does not have authentication token' do
        let(:printer_state) { OpenStruct.new(auth_token: nil) }
       
        context 'when server is reachable and printer is in home state' do

          it 'responds to registration code entry notification' do
            # Prepare responses to GetStatus command
            write_get_status_command_response_async(state: 'Home')
            
            # Client contacts server and receives registration code
            # Client subscribes to registration notification channel
            # Client commands smith to display registration code
            assert_primary_registration_code_sent_when_server_initially_reachable
            
            # User enters registration code into web portal and client receives notification from server
            # Client is notified of code entry by server and client commands smith to display registration success screen
            assert_primary_registration_succeeded_sent_when_notified_of_registration_code_entry
          end

        end

        context 'when server is not reachable initially but becomes reachable after retry interval' do

          it 'reattempts to reach server and responds to registration code entry notification' do
            write_get_status_command_response_async(state: 'Home')
            write_get_status_command_response_async(state: 'Home')
            
            # Simulate unreachable server by setting server url to invalid value
            set_settings_async(server_url: 'http://bad.url')
           
            # Need to to check the log so we can hook in to event loop to change server URL
            # TODO: might be a better way to do this with stubbed settings 
            assert_warn_log_entry_written_when_server_is_not_initially_reachable
            
            # Simulate server becoming reachable
            set_settings_async(server_url: dummy_server.url)

            # Client does nothing during retry interval
            # After retry interval client can reach server and receives registration code
            # Client subscribes to registration notification channel
            # Client commands smith to display registration code
            assert_primary_registration_code_sent
          end

        end

        context 'when printer is not in home state initially but is in home state after retry interval' do
          it 'reattempts to reach server and responds to registration code entry notification' do
            # Prepare responses to GetStatus command
            write_get_status_command_response_async(state: 'Printing')
            write_get_status_command_response_async(state: 'Home')

            assert_warn_log_entry_written_when_printer_not_initially_in_home_state

            # Client does nothing during retry interval
            # After retry interval printer is in home state
            # Client subscribes to registration notification channel
            # Client commands smith to display registration code
            assert_primary_registration_code_sent
          end
        end

      end

      context 'when printer does have authentication token'
    end

    context 'when command pipe is not open for reading initially but is open after retry interval' do

      let(:printer_state) { OpenStruct.new(auth_token: nil) }

      it 'logs error, reattempts to reach server and responds to registration code entry notification' do
        # Prepare response for retry attempt
        write_get_status_command_response_async(state: 'Home')
        
        assert_error_log_entry_written_when_command_pipe_not_open

        # Open command pipe on reading end
        watch_command_pipe_async

        assert_primary_registration_code_sent
      end

    end

  end

end
