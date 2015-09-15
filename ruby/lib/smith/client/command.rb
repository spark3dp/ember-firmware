#  File: command.rb
#  Base class for commands
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

require 'smith/client/url_helper'

module Smith
  module Client
    class Command

      COMPLETED_ACK = 'completed'
      RECEIVED_ACK = 'received'
      FAILED_ACK = 'failed'

      include URLHelper
      include PayloadHelper
      
      def initialize(state, http_client, payload)
        @state, @http_client, @payload = state, http_client, payload
      end

      private 

      # Send an command acknowledgement post request state is the stage of the command acknowledgment
      # If only the state is specified, the message is nil
      # If a string is specified as the second argument, it is formatted as a log message using any additional arguments
      # If something other than a string is specified, it is used directly
      def acknowledge_command(state, *args)
        m = message(*args)
        request = @http_client.post(acknowledge_endpoint(@payload), command_payload(@payload.command, state, m, Printer.get_status))
        request.callback { Client.log_debug(LogMessages::ACKNOWLEDGE_COMMAND, @payload.command, @payload.task_id, state, m) }
      end

      # Get the message portion of the acknowledgment according to the specified arguments
      def message(*args)
        return                          if args.empty?
        return LogMessage.format(*args) if args.first.is_a?(String)
        return args.first
      end

    end
  end
end
