# This is the spec helper for Smith::Client end to end integration tests

require 'common_helper'
require 'smith/client'

# Enable/disable printing client log messages to stdout
$client_log_enable = false
# Enable/disable printing VCR log messages to stdout
$vcr_log_enable = false
# Enable/disable printing Faye log messages to stdout
$faye_log_enable = false

RSpec.configure do |config|
  config.include(DummyServerHelper, :client)
  config.include(ClientHelper, :client)
  config.include(VCRSteps, :vcr)

  config.before(:suite) do
    # Start the dummy server in child process once before all tests
    $dummy_server = DummyServer.new
    $dummy_server.start
  end

  config.after(:suite) do
    $dummy_server.stop
  end

  config.before(:each, :client) do
    # Start a watchdog timer to timeout any tests that don't finish in a resonable amount of time
    EM.next_tick do
      @watchdog_timer = EM.add_timer(4) { raise 'timeout waiting for test to run' }
    end
  end

  config.after(:each, :client) do
    # Cancel the watchdog timer when a test completes
    @watchdog_timer.cancel if @watchdog_timer
  end

  config.before(:all, :vcr) do
    VCR.configure do |c|
      c.cassette_library_dir = 'cassettes'
      c.hook_into :webmock
      c.ignore_hosts 'localhost', '127.0.0.1', 'bad.url'
      c.debug_logger = STDOUT if $vcr_log_enable
    end
  end

  config.before(:each, :vcr) do |example|
    insert_vcr_cassette(example.metadata[:full_description])
  end

  config.after(:each, :vcr) do
    eject_vcr_cassette
  end

end
