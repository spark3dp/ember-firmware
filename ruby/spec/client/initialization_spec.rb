#  File: initialization_spec.rb
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
    describe 'Printer web client when registered', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include ClientPrimaryRegistrationSteps

      context 'when auth token is valid' do

        before do
          set_client_state_async
          set_printer_status_async(test_printer_status_values)
        end

        it 'does not send primary registration commands' do
          assert_primary_registration_commands_not_sent_when_auth_token_is_known
        end

      end

      context 'when auth token is not valid' do
        it 'clears auth token and printer id and re-attempts primary registration' do
          set_client_state_async(auth_token: 'expired_auth_token', printer_id: 5)

          # Prepare responses to GetStatus command for validation during registration
          set_printer_status_async(test_printer_status_values)

          # Client attempts registration but gets 403 response
          # Client clears auth token and id
          # Client attempts registration again and succeeds
          assert_primary_registration_code_sent_when_server_initially_reachable
        end
      end

    end
  end
end
