#  File: status_monitor.rb
#  Makes HTTP requests to server with status updates
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

require 'smith/nonblocking_io'

module Smith
  module Client
    module StatusMonitor

      include URLHelper
      include PayloadHelper

      def initialize(state, http_client)
        @state, @http_client = state, http_client
      end

      def notify_readable
        NonblockingIO.new(@io).readlines_nonblock.each do |status_json|
          # Remove line break character and ensure that status update is valid JSON
          status = JSON.parse(status_json.chomp)

          Client.log_debug(LogMessages::RECEIVE_STATUS_UPDATE, status)

          # Only make a request is the id is known
          @http_client.post(status_endpoint, status_payload(status)) if @state.printer_id
        end
      end

    end
  end
end
