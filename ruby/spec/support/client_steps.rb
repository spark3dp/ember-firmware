require 'rspec/em'

ClientSteps = RSpec::EM.async_steps do

  def assert_primary_registration_code_sent_when_server_initially_reachable(&callback)
    add_command_pipe_expectation(callback) do |command|
      expect(command).to eq(Smith::Printer::Commands::DISPLAY_PRIMARY_REGISTRATION_CODE)
      expect(registration_file_contents).to eq(registration_code: '4321', registration_url: 'autodesk.com/spark')
    end

    start_client
  end

  def assert_primary_registration_succeeded_sent_when_notified_of_registration_code_entry(&callback)
    add_command_pipe_expectation(callback) do |command|
      expect(command).to eq(Smith::Printer::Commands::PRIMARY_REGISTRATION_SUCCEEDED)
    end

    # Simulate user entering registration code into portal
    dummy_server.post('/v1/user/printers', registration_code: '4321')
  end

  def assert_warn_log_entry_written_when_server_is_not_initially_reachable(&callback)
    add_warn_log_expectation(callback) do |line|
      expect(line).to match(/Unable to reach server \(http:\/\/bad.url\), retrying in #{retry_interval} seconds/)
    end
    
    start_client
  end

  def assert_primary_registration_code_sent(&callback)
    add_command_pipe_expectation(callback) do |command|
      expect(registration_file_contents).to eq(registration_code: '4321', registration_url: 'autodesk.com/spark')
      expect(command).to eq(Smith::Printer::Commands::DISPLAY_PRIMARY_REGISTRATION_CODE)
    end
  end

  def assert_warn_log_entry_written_when_printer_not_initially_in_home_state(&callback)
    add_warn_log_expectation(callback) do |line|
      expect(line).to match(/Printer state \(state: "Printing", substate: nil\) invalid, retrying in #{retry_interval} seconds/)
    end

    start_client
  end

  def assert_error_log_entry_written_when_command_pipe_not_open(&callback)
    add_error_log_expectation(callback) do |line|
      expect(line).to match(/Unable to communicate with printer: execution expired, retrying in #{retry_interval} seconds/)
    end

    start_client
  end

end
