require 'client_helper'

module Smith
  module Client
    describe 'Printer web client when firmware_upgrade command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include FirmwareUpgradeHelperAsync
      include FirmwareUpgradeCommandSteps

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
        setup_firmware_upgrade_async
      end

      context 'when upgrade package can be applied successfully' do

        context 'when firmware package url does not return a redirect' do
          it 'applies upgrade and acknowledges success' do
            assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds(
              dummy_server.valid_firmware_upgrade_package_url
            )
          end
        end

        context 'when firmware package url does return a redirect' do
          it 'applies upgrade and acknowledges success' do
            assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds(
              dummy_server.latest_firmware_redirect_url
            )
          end
        end

      end

      context 'when upgrade package cannot be applied successfully' do
        it 'acknowledges failure' do
          assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_upgrade_fails
        end
      end
      
      context 'when download of upgrade package fails' do
        it 'acknowledges failure' do
          assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_download_fails
        end
      end
      
    end
  end
end
