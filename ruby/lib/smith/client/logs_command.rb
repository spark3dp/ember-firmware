# Class to handle command that requests client to upload its logs
# This implementation uses AWS S3

require 'smith/logs'
require 'securerandom'
require 'aws-sdk-core'

module Smith
  module Client
    class LogsCommand < Command

      def handle
        acknowledge_command(:received)
        # Run the upload in another thread in EM threadpool
        upload = proc { upload_log_archive }
        callback = proc { |url| upload_successful(url) if url }
        EM.next_tick do
          EM.defer(upload, callback)
        end
      end

      private

      def upload_log_archive
        # Upload gzipped tar archive containing logs with random file name
        s3 = Aws::S3::Client.new(
          region: Settings.aws_region,
          access_key_id: Settings.aws_access_key_id,
          secret_access_key: Settings.aws_secret_access_key
        )

        response = s3.put_object(bucket: Settings.s3_log_bucket, key: "#{SecureRandom.uuid}.tar.gz", body: StringIO.new(Logs.get_archive))

        # Extract and return URL
        response.context.http_request.endpoint.to_s
      rescue StandardError => e
        Client.log_error("Failure handling log command:\n#{e.backtrace.first}: #{e.message} (#{e.class})\n#{e.backtrace.drop(1).map{|s| "\t#{s}"}.join("\n")}")
        acknowledge_command(:failed, "Failure handling log command: #{e.message} (#{e.class})")
        nil # Return nil so the callback will get nil as the parameter and determine that the upload failed
      end

      def upload_successful(uploaded_archive_url)
        # Upload succeeded
        Client.log_info("Successfully uploaded logs to #{uploaded_archive_url}")

        # Send acknowledgement to server with location of upload
        acknowledge_command(:completed, url: uploaded_archive_url)
      end

    end
  end
end
