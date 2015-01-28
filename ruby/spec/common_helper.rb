# This is configuration shared by all tests
require 'bundler/setup'
require 'tmpdir'

require 'smith/settings'
require 'smith/state'

if ENV['COVERAGE']
  require 'simplecov'
  SimpleCov.start
end

Dir[File.expand_path('../support/*.rb', __FILE__)].each { |f| require(f) }

# Honor environment override of communication timeout
# This is set below in a before each hook so the default override handling
# in the settings factory will not take effect
if timeout = ENV['SMITH_PRINTER_COMMUNICATION_TIMEOUT']
  printer_communication_timeout = timeout.to_f
else
  printer_communication_timeout = 0.01
end

# Allow overriding of command pipe read timeout in print_engine_helper.rb
if timeout = ENV['TEST_NAMED_PIPE_TIMEOUT']
  $test_named_pipe_timeout = timeout.to_f
else
  $test_named_pipe_timeout = 1.0
end

RSpec.configure do |config|
  config.alias_example_to(:scenario)

  config.include(FileHelper, :tmp_dir)
  config.include(FileHelperAsync, :tmp_dir_async)

  config.before(:each) do
    # Use small timeout during tests
    Smith::Settings.printer_communication_timeout = printer_communication_timeout
    Smith::Settings.wireless_connection_delay = 0
    Smith::Settings.wireless_connection_poll_interval = 0
    Smith::Settings.wireless_connection_timeout = 0.01
  end

  config.before(:each, :tmp_dir) do
    make_tmp_dir
  end

  config.after(:each, :tmp_dir) do
    remove_tmp_dir
  end

  config.before(:each, :tmp_dir_async) do
    make_tmp_dir_async
  end

  config.after(:each, :tmp_dir_async) do
    remove_tmp_dir_async
  end

  config.mock_with(:rspec) do |mocks|
    mocks.verify_partial_doubles = true
  end
  
  config.order = 'random'
end
