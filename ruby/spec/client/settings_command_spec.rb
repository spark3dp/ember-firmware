require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when settings command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include SettingsCommandSteps

      let(:test_settings) { { SETTINGS_ROOT_KEY => { 'ProjectorLEDCurrent' => 200 } } }

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
        create_print_data_dir_async
      end
      
      context 'when settings command succeeds' do

        it 'saves settings to file, and sends command to load settings' do

          assert_settings_command_handled_when_settings_command_received

        end
      end

      context 'when settings command fails' do

        it 'acknowledges error' do
          # Close the command pipe to simulate error condition
          close_command_pipe_async
          assert_error_acknowledgement_sent_when_settings_command_fails
        end

      end

    end
  end
end
