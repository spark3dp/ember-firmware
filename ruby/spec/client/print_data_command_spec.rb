require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when print_data command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include PrintDataCommandSteps

      let(:print_settings) { { SETTINGS_ROOT_KEY => { 'JobName' => 'my print' } } }
      # The downloaded print file is named according to the URL it was downloaded from
      let(:test_print_file) { dummy_server.test_print_file_url.split('/').last }

      before do
        allow_primary_registration
        create_print_data_dir_async
      end
      
      context 'when print file specified by command is not the same as the print file currently loaded' do
        
        before { set_smith_settings_async(PRINT_FILE_SETTING => '') }

        context 'when printer is in valid state before and after downloading' do
        
          let(:stray_print_file) { File.join(print_data_dir, 'old.tar.gz') }
          
          it 'downloads file from specified url to download directory, saves settings to file, and sends commands to process data and load settings' do
            # GetStatus response used to validate that a download can start (no use downloading if the validation after the download completes will fail)
            write_get_status_command_response_async(state: HOME_STATE, substate: NO_SUBSTATE)
            # GetStatus response used to validate show loading command
            write_get_status_command_response_async(state: HOME_STATE, substate: NO_SUBSTATE)
            # GetStatus response used to validate process print data command
            write_get_status_command_response_async(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)

            touch_stray_print_file
            assert_print_data_command_handled_when_print_data_command_received_when_file_not_already_loaded_when_print_data_load_succeeds
            assert_print_data_dir_purged_before_print_file_download
          end

        end

        context 'when printer is in valid state before downloading but not in valid state after download is complete' do

          it 'acknowledges error' do
            # GetStatus response used to validate that a download can start (no use downloading if the validation after the download completes will fail)
            write_get_status_command_response_async(state: HOME_STATE, substate: NO_SUBSTATE)
            # GetStatus response used to validate show loading command
            write_get_status_command_response_async(state: HOME_STATE, substate: NO_SUBSTATE)
            # GetStatus response that causes print data command validation to fail
            write_get_status_command_response_async(state: HOME_STATE, substate: PRINT_DATA_LOAD_FAILED_SUBSTATE)

            assert_error_acknowledgement_sent_when_print_data_command_received_when_printer_not_in_valid_state_after_download
          end

        end

        context 'when printer is not in valid state before downloading' do

          it 'acknowledges error and does not download print data file' do
            # GetStatus response that causes download validation to fail
            write_get_status_command_response_async(state: PRINTING_STATE, substate: NO_SUBSTATE)

            assert_error_acknowledgement_sent_when_print_data_command_received
            assert_print_file_not_downloaded
          end

        end

        context 'when print file download fails' do

          it 'acknowledges error and sends show downloading failed command' do
            # GetStatus response used to validate that a download can start (no use downloading if the validation after the download completes will fail)
            write_get_status_command_response_async(state: HOME_STATE, substate: NO_SUBSTATE)

            assert_error_acknowledgement_sent_when_print_data_command_received_when_download_fails
          end

        end

      end

      context 'when print file specified by command is the same as the print file currently loaded' do

        let(:print_file_name) { 'test_print_file_already_downloaded' }

        before { set_smith_settings_async(PRINT_FILE_SETTING => print_file_name) }

        context 'when printer is in valid state' do

          it 'saves settings to file and sends commands to apply settings and indicate that print file is loaded' do
            # GetStatus response used to validate show loading command
            write_get_status_command_response_async(state: HOME_STATE, substate: NO_SUBSTATE)
            # GetStatus response used to validate apply print settings command
            write_get_status_command_response_async(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)
            # GetStatus response used to validate show loaded command
            write_get_status_command_response_async(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)

            assert_print_data_command_handled_when_print_data_command_received_when_file_already_loaded_when_load_settings_succeeds
          end

        end

        context 'when printer is not in valid state' do

          it 'acknowledges error' do
            write_get_status_command_response_async(state: HOME_STATE, substate: LOADING_PRINT_DATA_SUBSTATE)

            assert_print_data_command_handled_when_print_data_command_received_when_file_already_loaded_when_printer_not_in_valid_state
          end

        end

      end

    end
  end
end
