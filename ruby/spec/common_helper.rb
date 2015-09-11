#  File: common_helper.rb
#  Configuration shared by all tests
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
