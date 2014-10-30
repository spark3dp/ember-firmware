require 'client_helper'

module Smith
  describe 'Printer web client when print_data command is received', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include PrintDataCommandSteps

    before do
      allow_primary_registration
      create_print_data_dir_async
    end
    
    context 'when printer is in valid state' do
      let(:stray_print_file) { File.join(print_data_dir, 'old.tar.gz') }

      it 'downloads file from specified url to download directory, saves settings to file, and sends commands to process data and load settings' do
        # Prepare get status command responses for first two get status commands
        write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)
        write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)

        touch_stray_print_file
        assert_print_data_command_handled_when_print_data_command_received_when_print_data_load_succeeds
        assert_print_data_dir_purged_before_print_file_download
      end

    end

    context 'when printer is in valid state before downloading but not in valid state after download is complete' do

      it 'logs error' do
        # Prepare get status command responses for first two commands
        write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)
        write_get_status_command_response_async(state: Smith::HOME_STATE, substate: Smith::NO_SUBSTATE)

        assert_error_log_entry_written_when_print_data_command_received_when_printer_not_in_valid_state_after_download
      end

    end

    context 'when printer is not in valid state before downloading' do

      it 'logs error and does not download print data file' do
        # Prepare get status command response indicating that printer is not in home state
        write_get_status_command_response_async(state: Smith::PRINTING_STATE, substate: Smith::NO_SUBSTATE)

        assert_error_log_entry_written_and_data_not_downloaded_when_print_data_command_received
      end

    end

  end
end
