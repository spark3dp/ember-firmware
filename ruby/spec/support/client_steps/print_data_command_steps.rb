require 'fileutils'

PrintDataCommandSteps = RSpec::EM.async_steps do
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
      expect(payload[:request_params][:state]).to eq('received')
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
end
