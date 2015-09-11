#  File: request_settings_command.rb
#  Handles request settings command by posting current settings to server
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

module Smith
  module Client

    REQUEST_SETTINGS_COMMAND = 'request_settings'

    class RequestSettingsCommand < Command

      def handle
        acknowledge_command(Command::RECEIVED_ACK)
        # Send acknowledgement with settings in the payload
        acknowledge_command(Command::COMPLETED_ACK, Printer.settings)
      rescue StandardError => e
        Client.log_error(LogMessages::REQUEST_SETTINGS_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, nil, LogMessages::EXCEPTION_BRIEF, e)
      end

      # Send a command acknowledgement that can include the settings.
      # Post request state is the stage of the command acknowledgment
      # If only the state is specified, the message is nil
      # If a string is specified as the third argument, it is formatted as a log message using any additional arguments
      # If something other than a string is specified, it is used directly
      def acknowledge_command(state, settings = nil, *args)
        m = message(*args)
        cp = command_payload(@payload.command, state, m, Printer.get_status)
        if settings
          cp[:data].merge!({ settings: settings })
        end
        request = @http_client.post(acknowledge_endpoint(@payload), cp)
        request.callback { Client.log_debug(LogMessages::ACKNOWLEDGE_COMMAND, @payload.command, @payload.task_id, state, m) }
      end

    end
  end
end
