require 'client_helper'

module Smith
  describe 'Printer web client when firmware_upgrade command is received', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include FirmwareUpgradeCommandSteps

    before { allow_primary_registration }

    context 'when firmware package url returns a redirect' do
    end

    context 'when upgrade package can be applied successfully' do

      context 'when firmware package url does not return a redirect' do
        it 'applies upgrade and acknowledges success' do
          assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds(
            "#{dummy_server.url}/test_firmware_upgrade_package")
        end
      end

      context 'when firmware package url does return a redirect' do
        it 'applies upgrade and acknowledges success' do
          assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds("#{dummy_server.url}/redirect")
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
