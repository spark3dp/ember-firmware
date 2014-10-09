require 'rspec/em'
require 'zlib'
require 'json'
require 'rubygems/package'
require 'fileutils'

ClientCommandSteps = RSpec::EM.async_steps do

  def assert_command_forwarded_to_command_pipe(expected_command, &callback)
    add_command_pipe_expectation do |command|
      expect(command).to eq(expected_command)
      callback.call
    end

    dummy_server.post('/command', command: expected_command)
  end

  def assert_print_data_command_handled_when_print_data_command_received_when_print_data_load_succeeds(&callback)
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PRINT_DATA_LOAD)
    end
    
    write_get_status_command_response(state: Smith::HOME_STATE, substate: Smith::DOWNLOADING_SUBSTATE)
    expect_get_status_command
    
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PROCESS_PRINT_DATA)
      expect(File.read(File.join(print_data_dir, 'test_print_file'))).to eq("test print file contents\n")
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_APPLY_PRINT_SETTINGS)
      expect(print_settings_file_contents).to eq('JobName' => 'my print')
      callback.call
    end

    dummy_server.post('/command', command: 'print_data', file_url: "#{dummy_server.url}/test_print_file", settings: { 'JobName' => 'my print' }.to_json)
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

  def assert_error_log_entry_written_when_print_data_command_received_when_print_data_load_fails(&callback)
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PRINT_DATA_LOAD)
    end
    
    write_get_status_command_response(state: Smith::HOME_STATE, substate: Smith::DOWNLOAD_FAILED_SUBSTATE)
    expect_get_status_command

    add_error_log_expectation do |line|
      expect(line).to match(/Printer state \(state: "#{Smith::HOME_STATE}", substate: "#{Smith::DOWNLOAD_FAILED_SUBSTATE}"\) invalid, aborting print_data command handling/)
      callback.call
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
    # Subscribe to the test channel to receive notification of client posting URL of uploaded logs to server
    subscription = Faye::Client.new("#{dummy_server.url}/faye").subscribe('/test') do |raw_payload|
      payload = JSON.parse(raw_payload, symbolize_names: true)

      # Verify that the client made a request to the correct endpoint
      expect(payload[:command]).to eq('logs')
      expect(payload[:printer_id]).to eq('539')
      expect(payload).to have_key(:url)

      # Download the log file from S3 into an IO object
      log_archive_name = payload[:url].split('/').last
      log_archive_object = s3.buckets[bucket.name].objects[log_archive_name]
      log_archive = StringIO.new(log_archive_object.read)

      # Extract the archive and verify the contents
      tar_reader = Gem::Package::TarReader.new(Zlib::GzipReader.new(log_archive))
      tar_reader.rewind
      tar_reader.each do |entry|
        # If everything worked, this is the file that was written below!
        expect(entry.read).to eq('log file contents')
      end
      tar_reader.close

      # Delete the S3 bucket
      bucket.delete!

      callback.call
    end

    subscription.callback do
      # Create S3 bucket
      s3.buckets.create(bucket.name)

      # Create a sample log file
      File.write(tmp_dir('syslog'), 'log file contents')

      dummy_server.post('/command', command: 'logs')
    end

    subscription.errback { raise 'error subscribing to test channel in logs command test step' }
  end

  def assert_error_log_entry_written_when_log_upload_fails(&callback)
    add_error_log_expectation do |line|
      expect(line).to match(/Error uploading log archive/)
      callback.call
    end

    dummy_server.post('/command', command: 'logs')
  end

end
