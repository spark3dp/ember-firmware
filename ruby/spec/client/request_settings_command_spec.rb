require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when request_settings command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include RequestSettingsCommandSteps

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
        set_smith_settings_async('ProjectorLEDCurrent' => 200, 'InspectionHeightMicrons' => 65000)
      end
      
      context 'when request settings command succeeds' do

        it 'reads settings file, and returns settings in command acknowledgement payload' do

          assert_request_settings_command_handled_when_request_settings_command_received

        end
      end

      context 'when settings command fails' do

        it 'acknowledges error' do
          # delete the settings file to simulate error condition
          delete_smith_settings_async
          assert_error_acknowledgement_sent_when_request_settings_command_fails
        end

      end

    end
  end
end
