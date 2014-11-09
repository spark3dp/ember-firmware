require 'fileutils'

PrintDataCommandSteps = RSpec::EM.async_steps do

  def assert_print_data_command_handled_when_print_data_command_received_when_print_data_load_succeeds(&callback)
    # Use multiple deferrables to wait in this step until all expected notifications are received
    d1, d2, d3 = multi_deferrable(3) do
      callback.call
    end

    acknowledgement_notifications = []

    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PRINT_DATA_LOAD)
      d1.succeed
    end
    
    write_get_status_command_response(state: Smith::HOME_STATE, substate: Smith::LOADING_PRINT_DATA_SUBSTATE)
    expect_get_status_command
    
    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PROCESS_PRINT_DATA)
      expect(File.read(File.join(print_data_dir, 'test_print_file'))).to eq("test print file contents\n")
      expect(print_settings_file_contents).to eq(Smith::SETTINGS_ROOT_KEY => { 'JobName' => 'my print' })
      d2.succeed
    end

    subscription = subscribe_to_test_channel do |payload|
      acknowledgement_notifications.push(payload)
      
      # Wait until 2 ack notification are received
      if acknowledgement_notifications.length == 2
        
        received_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'received' }.first
        completed_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'completed' }.first

        # Verify acknowledgements
        expect(received_ack).not_to be_nil
        expect(received_ack[:request_params][:command]).to eq('print_data')
        expect(received_ack[:request_params][:command_token]).to eq('123456')
        assert_endpoint_match(payload[:request_endpoint], :acknowledge_endpoint)

        expect(completed_ack).not_to be_nil
        expect(completed_ack[:request_params][:command]).to eq('print_data')
        expect(completed_ack[:request_params][:command_token]).to eq('123456')
        assert_endpoint_match(payload[:request_endpoint], :acknowledge_endpoint)

        subscription.cancel
        d3.succeed
      end
    end

    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'print_data',
        command_token: '123456',
        file_url: "#{dummy_server.url}/test_print_file",
        settings: { Smith::SETTINGS_ROOT_KEY => { 'JobName' => 'my print' } }
      )
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

  def assert_error_acknowledgement_sent_when_print_data_command_received_when_printer_not_in_valid_state_after_download(&callback)
    d1, d2 = multi_deferrable(2) do
      callback.call
    end

    add_command_pipe_expectation do |command|
      expect(command).to eq(Smith::CMD_PRINT_DATA_LOAD)
      d1.succeed
    end

    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq('print_data')
      expect(payload[:request_params][:command_token]).to eq('123456')
      assert_endpoint_match(payload[:request_endpoint], :acknowledge_endpoint)

      subscription.cancel
      d2.succeed
    end
    
    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'print_data',
        command_token: '123456',
        file_url: "#{dummy_server.url}/test_print_file",
        settings: { Smith::SETTINGS_ROOT_KEY => { 'JobName' => 'my print' } }
      )
    end

    write_get_status_command_response(state: Smith::HOME_STATE, substate: Smith::PRINT_DATA_LOAD_FAILED_SUBSTATE)
    expect_get_status_command
  end

  def assert_error_acknowledgement_sent_when_print_data_command_received(&callback)
    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq('print_data')
      expect(payload[:request_params][:command_token]).to eq('123456')
      assert_endpoint_match(payload[:request_endpoint], :acknowledge_endpoint)

      subscription.cancel
      callback.call
    end

    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'print_data',
        command_token: '123456',
        file_url: "#{dummy_server.url}/test_print_file",
        settings: { Smith::SETTINGS_ROOT_KEY => { 'JobName' => 'my print' } }
      )
    end
  end

  def assert_print_file_not_downloaded(&callback)
    expect(File.exist?(File.join(print_data_dir, 'test_print_file'))).to be(false)
    callback.call
  end

  def assert_error_acknowledgement_sent_when_print_data_command_received_when_download_fails(&callback)
    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq('print_data')
      expect(payload[:request_params][:command_token]).to eq('123456')
      assert_endpoint_match(payload[:request_endpoint], :acknowledge_endpoint)

      subscription.cancel
      callback.call
    end

    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'print_data',
        command_token: '123456',
        file_url: "#{dummy_server.url}/bad",
        settings: { Smith::SETTINGS_ROOT_KEY => { 'JobName' => 'my print' } }
      )
    end
  end

end
