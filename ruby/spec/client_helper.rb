#  File: client_helper.rb
#  Spec helper for Smith::Client end-to-end integration tests
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'rspec/em'

require 'common_helper'
require 'smith/client'

# Require the steps
Dir[File.expand_path('../support/client_steps/*.rb', __FILE__)].each { |f| require(f) }

# Enable/disable printing Faye log messages to stdout
$faye_log_enable = false

# Enable/disable printing client log messages to stdout
if ENV['CLIENT_LOG_ENABLE']
  $client_log_enable = true
else
  $client_log_enable = false
end

# Allow overriding of per test timeout
if timeout = ENV['CLIENT_TEST_TIMEOUT']
  client_test_timeout = timeout.to_f
else
  client_test_timeout = 4.0
end

# Allow overriding of expectation timeouts
# This is used for timing out when expected HTTP requests and command pipe writes are
# not observed in dummy_server_helper.rb and print_engine_helper_async.rb
if timeout = ENV['CLIENT_EXPECTATION_TIMEOUT']
  $client_expectation_timeout = timeout.to_f
else
  $client_expectation_timeout = 2.0
end

RSpec.configure do |config|
  config.include(DummyServerHelper, :client)
  config.include(ClientHelper, :client)
  config.include(Smith::Client::URLHelper, :client)
  config.include(Smith::Client::PayloadHelper, :client)

  config.before(:suite) do
    # Start the dummy server in child process once before all tests
    $dummy_server = DummyServer.new
    $dummy_server.start
  end

  config.after(:suite) do
    $dummy_server.stop
  end

  config.before(:each, :client) do
    # Start a watchdog timer to timeout any tests that don't finish in a reasonable amount of time
    EM.next_tick do
      @watchdog_timer = EM.add_timer(client_test_timeout) { raise 'timeout waiting for test to run' }
    end
  end

  config.after(:each, :client) do
    # Cancel the watchdog timer when a test completes
    @watchdog_timer.cancel if @watchdog_timer
  end

end
