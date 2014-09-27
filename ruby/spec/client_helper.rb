# This is the spec helper for Smith::Client end to end integration tests

require 'rspec/em'
require 'common_helper'
require 'smith/client'
require 'timeout'

RSpec.configure do |config|
  config.include(DummyServerHelper, :client)
  config.include(ClientHelper, :client)
  
  config.before(:all, :client) do
    # Start the dummy server in child process once before all tests
    dummy_server.start
  end

  config.after(:all, :client) do
    dummy_server.stop
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
end
