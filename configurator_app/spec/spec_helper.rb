ENV['RACK_ENV'] = 'test'

require File.expand_path('../../app.rb', __FILE__)
require 'capybara'
require 'capybara/dsl'

Capybara.app = ConfiguratorApp.new

RSpec.configure do |config|
  config.alias_example_to(:scenario)
  config.treat_symbols_as_metadata_keys_with_true_values = true
  config.order = 'random'
  config.include Capybara::DSL
end