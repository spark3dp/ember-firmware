require 'zlib'
require 'rubygems/package'

module Smith
  module Client
    LogsCommandSteps = RSpec::EM.async_steps do

      def assert_logs_command_handled_when_logs_command_received(&callback)
        # Subscribe to the test channel to receive notification of client posting command acknowledgement
        allow(SecureRandom).to receive(:uuid).and_return('logs')
       
        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)

          # Download the log file from S3 into an IO object
          log_archive_name = request_params[:data][:message][:url].split('/').last
          log_archive = s3.get_object(bucket: bucket_name, key: log_archive_name).body

          # Extract the archive and verify the contents
          tar_reader = Gem::Package::TarReader.new(Zlib::GzipReader.new(log_archive))
          tar_reader.rewind
          tar_reader.each do |entry|
            # If everything worked, this is the file that was written below!
            expect(entry.read).to eq('log file contents')
          end
          tar_reader.close

          # Delete the object and bucket
          s3.delete_object(bucket: bucket_name, key: log_archive_name)
          s3.delete_bucket(bucket: bucket_name)
        end

        when_succeed(d1, d2) { callback.call }

        # Create S3 bucket
        s3.create_bucket(bucket: bucket_name)

        # Create a sample log file
        File.write(tmp_dir('syslog'), 'log file contents')

        dummy_server.post_command(command: LOGS_COMMAND, task_id: test_task_id)

      end

      def assert_error_acknowledgement_sent_when_log_command_handling_fails_fails(&callback)
        # Stub random uuid generator to return known value
        allow(SecureRandom).to receive(:uuid).and_return('logs')

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
        end
        
        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
          expect(request_params[:data][:message]).to match_log_message(
            LogMessages::EXCEPTION_BRIEF,
            Aws::S3::Errors::InvalidAccessKeyId.new('', '')
          )
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(command: LOGS_COMMAND, task_id: test_task_id)
      end

    end
  end
end
