# Class to handle command that requests client to upload its logs
# This implementation uses AWS S3

require 'smith/logs'
#require 'securerandom'
require 'aws-sdk'

module Smith
  module Client
    class LogsCommand

      def initialize(printer, payload, printer_id)
        @printer_id = printer_id
      end

      def handle
        # Run the upload in another thread in EM threadpool
        upload = proc { upload_log_archive }
        callback = proc { |o| upload_successful(o) if o }
        EM.defer(upload, callback)
      end

      private

      def upload_log_archive
        # Upload gzipped tar archive containing logs with random file name
        # TODO: make log archive name unique
        s3 = AWS::S3.new(access_key_id: Client.aws_access_key_id, secret_access_key: Client.aws_secret_access_key)
        s3.buckets[Client.s3_log_bucket].objects['logs.tar.gz'].write(Logs.get_archive)
      rescue StandardError => e
        Client.log_error("Error uploading log archive (#{e.class}: #{e.message})")
        nil # Return nil so the callback will get nil as the parameter and determine that the upload failed
      end

      def upload_successful(s3_object)
        # Upload succeeded, object is the S3 object
        Client.log_info("Successfully uploaded logs to #{s3_object.public_url}")

        # Make request to server with location of upload
        request = EM::HttpRequest.new("#{Client.server_url}/#{Client.api_version}/printers/#{@printer_id}/logs").post(
          head: { 'Content-Type' => 'application/json' },
          body: { url: s3_object.public_url }.to_json
        )

        request.callback { Client.log_debug("Successfully made request to server with location of uploaded logs") }
        request.errback { Client.log_error("Error making request to server in response to logs command") }
      end

    end
  end
end
