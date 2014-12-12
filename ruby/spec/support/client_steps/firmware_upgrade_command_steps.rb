FirmwareUpgradeCommandSteps = RSpec::EM.async_steps do

  def assert_firmware_upgrade_command_handled_when_firmware_upgrade_command_received_when_upgrade_succeeds(url, &callback)
    upgrade_called = false

    allow(Smith::Config::Firmware).to receive(:upgrade) do |upgrade_package|
      expect(File.read(upgrade_package)).to eq("test firmware upgrade package contents\n")
      upgrade_called = true
    end
  
    d1 = add_http_request_expectation acknowledge_endpoint do |request_params|
      expect(request_params[:command]).to eq('firmware_upgrade')
      expect(request_params[:command_token]).to eq('123456')
      expect(request_params[:state]).to eq('received')
    end
    
    d2 = add_http_request_expectation acknowledge_endpoint do |request_params|
      expect(request_params[:command]).to eq('firmware_upgrade')
      expect(request_params[:command_token]).to eq('123456')
      expect(request_params[:state]).to eq('completed')

      # Check that the upgrade was actually preformed
      expect(upgrade_called).to eq(true)
    end

    when_succeed(d1, d2) { callback.call }

    dummy_server.post(
      '/command',
      command: 'firmware_upgrade',
      command_token: '123456',
      package_url: url 
    )
  end

  def assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_upgrade_fails(&callback)
    allow(Smith::Config::Firmware).to receive(:upgrade).and_raise(Smith::Config::Firmware::UpgradeError, 'the error message')

    d1 = add_http_request_expectation acknowledge_endpoint do |request_params|
      expect(request_params[:command]).to eq('firmware_upgrade')
      expect(request_params[:command_token]).to eq('123456')
      expect(request_params[:state]).to eq('received')
    end
    
    d2 = add_http_request_expectation acknowledge_endpoint do |request_params|
      expect(request_params[:state]).to eq('failed')
      expect(request_params[:command]).to eq('firmware_upgrade')
      expect(request_params[:command_token]).to eq('123456')
      expect(request_params[:message]).to eq('the error message (Smith::Config::Firmware::UpgradeError)')
    end

    when_succeed(d1, d2) { callback.call }

    dummy_server.post(
      '/command',
      command: 'firmware_upgrade',
      command_token: '123456',
      package_url: "#{dummy_server.url}/test_firmware_upgrade_package"
    )
  end

  def assert_failure_acknowledgement_sent_when_firmware_upgrade_command_received_when_download_fails(&callback)
    d1 = add_http_request_expectation acknowledge_endpoint do |request_params|
      expect(request_params[:command]).to eq('firmware_upgrade')
      expect(request_params[:command_token]).to eq('123456')
      expect(request_params[:state]).to eq('received')
    end
    
    d2 = add_http_request_expectation acknowledge_endpoint do |request_params|
      expect(request_params[:state]).to eq('failed')
      expect(request_params[:command]).to eq('firmware_upgrade')
      expect(request_params[:command_token]).to eq('123456')
      expect(request_params[:message]).to eq("Error downloading firmware package from \"#{dummy_server.url}/bad\"")
    end

    when_succeed(d1, d2) { callback.call }
    
    dummy_server.post(
      '/command',
      command: 'firmware_upgrade',
      command_token: '123456',
      package_url: "#{dummy_server.url}/bad"
    )
  end

end
