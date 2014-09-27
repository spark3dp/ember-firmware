# Extension for faye client to add authentication token to outgoing messages

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
