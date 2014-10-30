require 'zlib'
require 'rubygems/package'

LogsCommandSteps = RSpec::EM.async_steps do

  def assert_logs_command_handled_when_logs_command_received(&callback)
    # Subscribe to the test channel to receive notification of client posting command acknowledgement
    allow(SecureRandom).to receive(:uuid).and_return('logs')

    # Subscribe to the test channel to receive notification of client posting URL of uploaded logs to server
    subscription = subscribe_to_test_channel do |payload|
      # Verify that the client made acknowledgement request to the correct endpoint
      expect(payload[:request_params][:state]).to eq('completed')
      expect(payload[:request_params][:command]).to eq('logs')
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)

      # Download the log file from S3 into an IO object
      log_archive_name = payload[:request_params][:message][:url].split('/').last
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

      subscription.cancel
      callback.call
    end

    subscription.callback do
      # Create S3 bucket
      s3.create_bucket(bucket: bucket_name)

      # Create a sample log file
      File.write(tmp_dir('syslog'), 'log file contents')

      dummy_server.post('/command', command: 'logs', command_token: '123456')
    end

  end

  def assert_error_log_entry_written_when_log_upload_fails(&callback)
    # Stub random uuid generator to return known value
    allow(SecureRandom).to receive(:uuid).and_return('logs')

    add_error_log_expectation do |line|
      expect(line).to match(/Error uploading log archive/)
      callback.call
    end

    dummy_server.post('/command', command: 'logs', command_token: '123456')
  end

end
