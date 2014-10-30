require 'client_helper'

module Smith
  describe 'Printer web client when firmware_upgrade command is received', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include FirmwareUpgradeCommandSteps

    before { allow_primary_registration }

    context 'when upgrade succeeds' do
      it 'applies upgrade and acknowledges success' do
        assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds
      end
    end

    context 'when upgrade fails' do
      it 'acknowledges failure' do
        assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_upgrade_fails
      end
    end
    
    context 'when upgrade when download fails' do
      it 'acknowledges failure' do
        assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_download_fails
      end
    end
    
  end
end
