#  File: firmware_upgrade_command_spec.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
