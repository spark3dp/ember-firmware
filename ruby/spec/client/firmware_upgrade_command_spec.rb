require 'client_helper'

module Smith
  describe 'Printer web client when firmware_upgrade command is received', :client do
    # See support/client_context.rb for setup/teardown
    include ClientSteps
    include FirmwareUpgradeCommandSteps

    before { allow_primary_registration }

    context 'when upgrade package is valid' do
      it 'applys upgrade and acknowledges success' #do
      #  assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_firmware_package_is_valid
      #end
    end

    context 'when upgrade package is not valid' do
      it 'acknowledges failure' #do
      #  assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_firmware_package_is_not_valid
      #end
    end

  end
end
