# Class to handle command that requests client to upload its logs
# This implementation uses AWS S3

require 'smith/logs'
require 'securerandom'
require 'aws-sdk-core'

module Smith
  module Client
    class LogsCommand

      include URLHelper

      def initialize(printer, state, payload)
        @state = state
        @command, @command_token = payload.values_at :command, :command_token
      end

      def handle
        # Run the upload in another thread in EM threadpool
        upload = proc { upload_log_archive }
        callback = proc { |url| upload_successful(url) if url }
        EM.defer(upload, callback)
      end

      private

      def upload_log_archive
        # Upload gzipped tar archive containing logs with random file name
        s3 = Aws::S3::Client.new(region: Settings.aws_region, access_key_id: Settings.aws_access_key_id, secret_access_key: Settings.aws_secret_access_key)
        response = s3.put_object(bucket: Settings.s3_log_bucket, key: "#{SecureRandom.uuid}.tar.gz", body: StringIO.new(Logs.get_archive))
        # Extract and return URL
        response.context.http_request.endpoint.to_s
      rescue StandardError => e
        Client.log_error("Error uploading log archive (#{e.class}: #{e.message})")
        nil # Return nil so the callback will get nil as the parameter and determine that the upload failed
      end

      def upload_successful(uploaded_archive_url)
        # Upload succeeded, object is the S3 object
        Client.log_info("Successfully uploaded logs to #{uploaded_archive_url}")

        # Make request to server with location of upload
        request = EM::HttpRequest.new(acknowledge_endpoint(@state)).post(
          head: { 'Content-Type' => 'application/json' },
          body: { command: @command, command_token: @command_token, message: { url: uploaded_archive_url } }.to_json
        )

        request.callback { Client.log_debug("Successfully acknowledged #{@command.inspect} command (command token #{@command_token.inspect})") }
        request.errback { Client.log_error("Error making acknowledgement request to server in response to #{@command.inspect} command (command token #{@command_token.inspect})") }
      end

    end
  end
end
