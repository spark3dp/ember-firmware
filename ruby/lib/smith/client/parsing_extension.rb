#  File: parsing_extension.rb
#  Faye extension to convert incoming message data to a Hash with symbol keys
#  Correctly handles messages that arrive as JSON strings or Hashes with string keys
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

require 'json'

module Smith
  module Client
    class ParsingExtension

      def incoming(message, callback)

        # Do not modify messages on meta channels
        if message['channel'] =~ /\/meta\/.*?\z/
          return callback.call(message)
        end

        data = message['data']

        if data.is_a?(String)
          message['data'] = JSON.parse(data, symbolize_names: true)
        else
          message['data'] = JSON.parse(data.to_json, symbolize_names: true)
        end

        callback.call(message)
      end

    end
  end
end
