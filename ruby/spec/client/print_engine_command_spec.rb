#  File: print_engine_command_spec.rb
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
    describe 'Printer web client when print engine command is received', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include PrintEngineCommandSteps

      before do
        allow_primary_registration
        set_printer_status_async(test_printer_status_values)
      end

      context 'when incoming message is a Hash' do
        it 'forwards command to command pipe and sends command acknowledgements' do
          assert_command_acknowledged_and_forwarded_to_command_pipe_when_message_is_a_hash
        end
      end

      context 'when no errors are raised during command handling' do
        it 'forwards command to command pipe and sends command acknowledgements' do
          assert_command_acknowledged_and_forwarded_to_command_pipe(CMD_PAUSE)
        end
      end

      context 'when an error is raised during command handling' do
        it 'acknowledges error' do
          # Close the command pipe to simulate error condition
          close_command_pipe_async
          assert_error_acknowledgement_sent_when_print_engine_command_fails(CMD_PAUSE)
        end
      end

    end
  end
end
