#  File: authentication_extension.rb
#  Extension for faye client to add authentication token to outgoing messages
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

module Smith
  module Client
    class AuthenticationExtension

      def initialize(token)
        @authentication_token = token
      end

      def outgoing(message, callback)
        # Again, leave non-subscribe messages alone
        unless message['channel'] == '/meta/subscribe'
          return callback.call(message)
        end

        # Add ext field if it's not present
        message['ext'] ||= {}

        # Set the auth token
        message['ext']['authentication_token'] = @authentication_token

        # Carry on and send the message to the server
        callback.call(message)
      end

    end
  end
end
