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


RSpec.configure do |config|
  config.alias_example_to(:scenario)

  config.include(FileHelper, :tmp_dir)
  config.include(FileHelperAsync, :tmp_dir_async)

  config.before(:each) do
    # Set state file to temp path
    Smith::Settings.state_file = File.join(Dir.tmpdir,"#{Time.now.to_i}#{rand(1000)}settings")

    # Make sure state object is reset before each test
    Smith::State.load

    # Use small timeout during tests
    Smith::Settings.printer_communication_timeout = 0.01
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
