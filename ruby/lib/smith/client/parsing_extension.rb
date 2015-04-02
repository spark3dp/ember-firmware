# Faye extension to convert incoming message data to a Hash with symbol keys
# Correctly handles messages that arrive as JSON strings or Hashes with string keys

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
