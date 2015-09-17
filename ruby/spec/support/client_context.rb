#  File: client_context.rb
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

# Setup and tear down for client tests
# Automatically included in any example groups tagged with :client

module Smith
  module Client

    RSpec.shared_context 'client context', :client do
      include PrintEngineHelperAsync
      include LogHelperAsync

      let(:expected_registration_file_contents) { {
        REGISTRATION_CODE_KEY => dummy_server.registration_code,
        REGISTRATION_URL_KEY => dummy_server.registration_url
      } }

      before do
        set_test_specific_settings_async

        create_command_pipe_async
        create_printer_status_file_async
       
        # Create an in-memory state object for tests
        @state = InMemoryState.new

        # Open and watch command pipe
        watch_command_pipe_async

        # Set argument to true to print client log output to stdout
        watch_log_async($client_log_enable)
        
        # Server is reachable by default
        set_settings_async(server_url: dummy_server.url)

        # The dummy server publishes a notification on a test channel when it receives certain requests
        # Subscribe to this test channel so the tests can make assertions about requests the client makes
        subscribe_to_test_channel_async
      end

      after do
        # Cleanup
        unsubscribe_from_test_channel_async
        stop_client_async
        stop_watching_log_async
        close_command_pipe_async
      end
    end

  end
end
