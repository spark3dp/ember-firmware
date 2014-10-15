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
    if msg_token != 'authtoken'
      Rails.logger.info "\n\n******Faye subscription received with invalid auth token, expected authentication_token to equal 'authtoken'******"
      Rails.logger.info "Incoming message: #{message.inspect}\n\n"
      message['error'] = 'Invalid subscription auth token'
    end

    # Call the server back now we're done
    callback.call(message)
  end
end
