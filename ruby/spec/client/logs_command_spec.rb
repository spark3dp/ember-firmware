#  File: logs_command_spec.rb
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
    describe 'Printer web client when logs command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include LogsCommandSteps

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
      end

      context 'when upload is possible' do

        it 'uploads log archive and responds to server with location' do |example|
          assert_logs_command_handled_when_logs_command_received
        end

      end

      context 'when server returns status code indicating error' do

        it 'acknowledges error' do
          assert_error_acknowledgement_sent_when_log_command_fails_due_to_http_error_received
        end

      end

      context 'when server is not reachable' do

        it 'acknowledges error' do
          # Set timeout to smaller value so the test does not need to wait excessively
          # long before the client sends the failure acknowledgement
          set_settings_async(file_upload_connect_timeout: 0.1)
          assert_error_acknowledgement_sent_when_log_command_fails_due_to_unreachable_server
        end

      end

    end
  end
end
