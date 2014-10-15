require 'zlib'
require 'rubygems/package'
require 'fileutils'

ClientCommandSteps = RSpec::EM.async_steps do

  def assert_command_forwarded_to_command_pipe(expected_command, &callback)
    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(expected_command)
      d1.succeed
    end
    
    # Subscribe to the test channel to receive notification of client posting command acknowledgement
    subscription = subscribe_to_test_channel do |payload|
      expect(payload[:request_params][:command]).to eq(expected_command)
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)
      subscription.cancel
      d2.succeed
    end

    subscription.callback do
      dummy_server.post('/command', command: expected_command, command_token: '123456')
    end
  end

  def assert_print_data_command_handled_when_print_data_command_received_when_print_data_load_succeeds(&callback)
    # Use multiple deferrables to wait in this step until all expected notifications are received
    d1, d2, d3 = multi_deferrable(3) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PRINT_DATA_LOAD)
      d1.succeed
    end
    
    write_get_status_command_response(state: Smith::HOME_STATE, substate: Smith::DOWNLOADING_SUBSTATE)
    expect_get_status_command
    
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PROCESS_PRINT_DATA)
      expect(File.read(File.join(print_data_dir, 'test_print_file'))).to eq("test print file contents\n")
      expect(print_settings_file_contents).to eq(Smith::SETTINGS_ROOT_KEY => { 'JobName' => 'my print' })
      d2.succeed
    end

    subscription = subscribe_to_test_channel do |payload|
      expect(payload[:request_params][:command]).to eq('print_data')
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)
      subscription.cancel
      d3.succeed
    end

    subscription.callback do
      dummy_server.post('/command', command: 'print_data', command_token: '123456', file_url: "#{dummy_server.url}/test_print_file", settings: { 'JobName' => 'my print' })
    end
  end

  def assert_print_data_dir_purged_before_print_file_download(&callback)
    expect(File.exists?(stray_print_file)).to eq(false)
    callback.call
  end

  def touch_stray_print_file(&callback)
    # Create old print file that needs to be deleted before downloading the new file
    # A stray file might exist as a result of an error during print data processing
    FileUtils.touch(stray_print_file)
    callback.call
  end

  def assert_error_log_entry_written_when_print_data_command_received_when_printer_not_in_valid_state_after_download(&callback)
    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PRINT_DATA_LOAD)
      d1.succeed
    end
    
    write_get_status_command_response(state: Smith::HOME_STATE, substate: Smith::DOWNLOAD_FAILED_SUBSTATE)
    expect_get_status_command

    add_error_log_expectation do |line|
      expect(line).to match(/Printer state \(state: "#{Smith::HOME_STATE}", substate: "#{Smith::DOWNLOAD_FAILED_SUBSTATE}"\) invalid, aborting print_data command handling/)
      d2.succeed
    end
    
    dummy_server.post('/command', command: 'print_data', file_url: "#{dummy_server.url}/test_print_file", settings: { 'JobName' => 'my print' }.to_json)
  end

  def assert_error_log_entry_written_and_data_not_downloaded_when_print_data_command_received(&callback)
    add_error_log_expectation do |line|
      expect(File.exist?(File.join(print_data_dir, 'test_print_file'))).to be(false)
      expect(line).to match(
        /Printer state \(state: "#{Smith::PRINTING_STATE}", substate: "#{Smith::NO_SUBSTATE}"\) invalid, not downloading print data, aborting print_data command handling/)
      callback.call
    end
    
    dummy_server.post('/command', command: 'print_data', file_url: "#{dummy_server.url}/test_print_file", settings: { 'JobName' => 'my print' }.to_json)
  end

  def assert_logs_command_handled_when_logs_command_received(&callback)
    # Subscribe to the test channel to receive notification of client posting command acknowledgement
    allow(SecureRandom).to receive(:uuid).and_return('logs')

    # Subscribe to the test channel to receive notification of client posting URL of uploaded logs to server
    subscription = subscribe_to_test_channel do |payload|
      # Verify that the client made acknowledgement request to the correct endpoint
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
