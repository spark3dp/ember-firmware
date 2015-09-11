#  File: primary_registration_spec.rb
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
    describe 'Printer web client when started when not registered', :client do
      # See support/client_context.rb for setup/teardown
      include ClientPrimaryRegistrationSteps

      context 'when server is reachable and printer is in home state' do

        it 'responds to registration code entry notification' do
          set_printer_status_async(state: HOME_STATE)
          
          # Client contacts server and receives registration code
          # Client subscribes to registration notification channel
          # Client commands smith to display registration code
          assert_primary_registration_code_sent_when_server_initially_reachable
          
          # User enters registration code into web portal and client receives notification from server
          # Client is notified of code entry by server and client commands smith to display registration success screen
          assert_primary_registration_succeeded_sent_when_notified_of_registration_code_entry

          # Authentication token and printer id are persisted 
          assert_identity_persisted
        end

      end

      context 'when server is not reachable initially but becomes reachable after retry interval' do

        it 'reattempts to reach server and responds to registration code entry notification' do
          set_printer_status_async(state: HOME_STATE)
          
          # Server is initially unreachable
          # Simulate unreachable server by setting server url to invalid value
          set_settings_async(server_url: 'http://bad.url')
         
          # Client does nothing during retry interval
          # After retry interval client can reach server and receives registration code
          # Client subscribes to registration notification channel
          # Client commands smith to display registration code
          assert_registration_reattempted_after_server_not_reachable
          
          # Authentication token and printer id are persisted 
          assert_identity_persisted
        end

      end

      context 'when printer is not in home state initially but is in home state after retry interval' do
        it 'reattempts to reach server and responds to registration code entry notification' do
          # Initially not in valid state for registration
          set_printer_status_async(state: PRINTING_STATE)

          # First registration attempt fails since printer is not in home state
          # Client does nothing during retry interval
          # After retry interval printer is in home state
          # Client subscribes to registration notification channel
          # Client commands smith to display registration code
          assert_registration_reattempted_after_not_in_valid_state
         
          # Authentication token and printer id are persisted 
          assert_identity_persisted
        end

      end

    end

  end
end
