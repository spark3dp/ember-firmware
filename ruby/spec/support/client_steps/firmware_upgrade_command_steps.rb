FirmwareUpgradeCommandSteps = RSpec::EM.async_steps do

  def assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds(url, &callback)
    # Store the argument that the firmware upgrade method is called with and acknowledgement notifications
    upgrade_package = nil
    acknowledgement_notifications = []
    
    allow(Smith::Config::Firmware).to receive(:upgrade) { |arg| upgrade_package = arg }

    subscription = subscribe_to_test_channel do |payload|
      acknowledgement_notifications.push(payload)
      
      # Wait until 2 ack notification are received
      if acknowledgement_notifications.length == 2
        
        received_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'received' }.first
        completed_ack = acknowledgement_notifications.select { |r| r[:request_params][:state] == 'completed' }.first

        # Verify acknowledgements
        expect(received_ack).not_to be_nil
        expect(received_ack[:request_params][:command]).to eq('firmware_upgrade')
        expect(received_ack[:request_params][:command_token]).to eq('123456')
        expect(received_ack[:request_endpoint]).to match(/printers\/539\/acknowledge/)

        expect(completed_ack).not_to be_nil
        expect(completed_ack[:request_params][:command]).to eq('firmware_upgrade')
        expect(completed_ack[:request_params][:command_token]).to eq('123456')
        expect(completed_ack[:request_endpoint]).to match(/printers\/539\/acknowledge/)

        # Check that the upgrade was actually preformed
        expect(upgrade_package).not_to be_nil
        expect(File.read(upgrade_package)).to eq("test firmware upgrade package contents\n")

        subscription.cancel
        callback.call
      end
    end

    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'firmware_upgrade',
        command_token: '123456',
        package_url: url 
      )
    end
  end

  def assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_upgrade_fails(&callback)
    allow(Smith::Config::Firmware).to receive(:upgrade).and_raise(Smith::Config::Firmware::UpgradeError, 'the error message')

    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq('firmware_upgrade')
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_params][:message]).to eq('the error message (Smith::Config::Firmware::UpgradeError)')
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)

      subscription.cancel
      callback.call
    end

    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'firmware_upgrade',
        command_token: '123456',
        package_url: "#{dummy_server.url}/test_firmware_upgrade_package"
      )
    end
  end

  def assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_download_fails(&callback)
    subscription = subscribe_to_test_channel do |payload|
      # Only assert the failure notification
      next if payload[:request_params][:state] == 'received'

      expect(payload[:request_params][:state]).to eq('failed')
      expect(payload[:request_params][:command]).to eq('firmware_upgrade')
      expect(payload[:request_params][:command_token]).to eq('123456')
      expect(payload[:request_params][:message]).to eq("Error downloading firmware package from \"#{dummy_server.url}/bad\"")
      expect(payload[:request_endpoint]).to match(/printers\/539\/acknowledge/)

      subscription.cancel
      callback.call
    end

    subscription.callback do
      dummy_server.post(
        '/command',
        command: 'firmware_upgrade',
        command_token: '123456',
        package_url: "#{dummy_server.url}/bad"
      )
    end
  end

end
