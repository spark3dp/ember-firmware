require 'zlib'
require 'rubygems/package'

LogsCommandSteps = RSpec::EM.async_steps do

  def assert_logs_command_handled_when_logs_command_received(&callback)
    # Subscribe to the test channel to receive notification of client posting command acknowledgement
    allow(SecureRandom).to receive(:uuid).and_return('logs')
   
    acknowledgement_notifications = []

    # Subscribe to the test channel to receive notification of client posting URL of uploaded logs to server
    subscription = subscribe_to_test_channel do |payload|
      acknowledgement_notifications.push(payload)
      
      # Wait until 2 ack notification are received
      if acknowledgement_notifications.length == 2
        
        received_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'received' }.first
        completed_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'completed' }.first

        # Verify acknowledgements
        expect(received_ack).not_to be_nil
        expect(received_ack[:request_params][:command]).to eq('logs')
        expect(received_ack[:request_params][:command_token]).to eq('123456')
        expect(received_ack[:request_endpoint]).to match(/printers\/539\/acknowledge/)

        expect(completed_ack).not_to be_nil
        expect(completed_ack[:request_params][:command]).to eq('logs')
        expect(completed_ack[:request_params][:command_token]).to eq('123456')
        expect(completed_ack[:request_endpoint]).to match(/printers\/539\/acknowledge/)

        # Download the log file from S3 into an IO object
        log_archive_name = completed_ack[:request_params][:message][:url].split('/').last
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
    end

    subscription.callback do
      # Create S3 bucket
      s3.create_bucket(bucket: bucket_name)

      # Create a sample log file
      File.write(tmp_dir('syslog'), 'log file contents')

      dummy_server.post('/command', command: 'logs', command_token: '123456')
    end

  end

  def assert_error_acknowledgement_sent_when_log_command_handling_fails_fails(&callback)
    # Stub random uuid generator to return known value
    allow(SecureRandom).to receive(:uuid).and_return('logs')

    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq('logs')
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_params][:message]).to match(/Failure handling log command/)
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)

      subscription.cancel
      callback.call
    end

    subscription.callback do
      dummy_server.post('/command', command: 'logs', command_token: '123456')
    end
  end

end
