#  File: url_helper.rb
#  Mix in to provide helper methods for building URLs
#  Requires the including object to have an instance variable @state
#  that has a printer_id method for dynamic url generation
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

require 'erb'

module Smith
  module Client
    module URLHelper

      def client_endpoint
        "#{Settings.server_url}/#{Settings.client_endpoint}"
      end

      def registration_endpoint
        "#{Settings.server_url}/#{Settings.server_api_namespace}/#{Settings.registration_endpoint}"
      end

      def acknowledge_endpoint(context)
        interpolate("#{Settings.server_url}/#{Settings.server_api_namespace}/#{Settings.acknowledge_endpoint}", context)
      end
      
      def status_endpoint
        interpolate("#{Settings.server_url}/#{Settings.server_api_namespace}/#{Settings.status_endpoint}", @state)
      end

      def registration_channel
        interpolate(Settings.registration_channel, @state)
      end

      def command_channel
        interpolate(Settings.command_channel, @state)
      end

      def interpolate(template, context)
        ERB.new(template).result(context.get_binding)
      end

    end
  end
end
