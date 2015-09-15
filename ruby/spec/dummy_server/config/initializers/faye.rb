#  File: faye.rb
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

#Faye.logger = Rails.logger

# Define server-side auth extension
class ServerAuth
  def incoming(message, callback)
    # Let non-subscribe messages through
    unless message['channel'] == '/meta/subscribe'
      return callback.call(message)
    end

    # Get subscribed channel and auth token
    subscription = message['subscription']
    msg_token    = message['ext'] && message['ext']['authentication_token']

    # Let test messages through
    if subscription == '/test'
      return callback.call(message)
    end

    # Add an error if correct token is not provided
    if msg_token != AUTH_TOKEN
      Rails.logger.info "\n\n******Faye subscription received with invalid auth token, expected authentication_token to equal #{AUTH_TOKEN.inspect}******"
      Rails.logger.info "Incoming message: #{message.inspect}\n\n"
      message['error'] = 'Invalid subscription auth token'
    end

    # Call the server back now we're done
    callback.call(message)
  end
end
