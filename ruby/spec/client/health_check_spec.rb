#  File: health_check_spec.rb
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
    describe 'Printer web client when running', :client do
      # See support/client_context.rb for setup/teardown
      include ClientSteps
      include ClientHealthCheckSteps

      # Set interval to small value so tests don't take unnecessarily long
      # Keep interval greater than zero to avoid making many requests
      let(:periodic_status_interval) { 0.05 }

      before do
        set_client_state_async
        set_printer_status_async(test_printer_status_values)
      end

      it 'makes periodic health check requests to server' do
        # assert that client makes two health check requests
        assert_periodic_health_checks_made_when_running
      end

      context 'when server becomes temporarily unreachable' do
        it 'continues making health check requests after re-establishing contact with server' do
          assert_error_logged_and_health_checks_resume_after_temporary_connection_loss
        end
      end

    end
  end
end
