#  File: request_settings_command_spec.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Richard Greene
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
