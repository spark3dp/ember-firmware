# Class to handle command that requests client to upload its logs
# This implementation uses AWS S3

require 'smith/logs'

module Smith
  module Client
  
    LOGS_COMMAND = 'logs'

    class LogsCommand < Command

      def handle
        acknowledge_command(Command::RECEIVED_ACK)

        Client.log_info(LogMessages::START_LOG_UPLOAD, @payload.upload_url)

        request = EM::HttpRequest.new(
          @payload.upload_url,
          connect_timeout: Settings.file_upload_connect_timeout,
          inactivity_timeout: Settings.file_upload_inactivity_timeout
        ).put(head: { 'Content-Type' => 'application/gzip' }, body: Logs.get_archive)

        request.callback do
          header = request.response_header
          if header.successful?
            # Upload succeeded
            Client.log_info(LogMessages::LOG_UPLOAD_SUCCESS, @payload.upload_url)
            acknowledge_command(Command::COMPLETED_ACK)
          else
            # Server reached but response did not indicate success
            Client.log_error(LogMessages::LOG_UPLOAD_HTTP_ERROR, @payload.upload_url, header.status)
            acknowledge_command(Command::FAILED_ACK, LogMessages::LOG_UPLOAD_HTTP_ERROR, @payload.upload_url, header.status)
          end 
        end

        request.errback do
          # Unable to reach specified URL
          Client.log_error(LogMessages::LOG_UPLOAD_URL_UNREACHABLE, @payload.upload_url)
          acknowledge_command(Command::FAILED_ACK, LogMessages::LOG_UPLOAD_URL_UNREACHABLE, @payload.upload_url)
        end
      end

    end
  end
end
