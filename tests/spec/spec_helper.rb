require 'bundler/setup'
Bundler.setup

RSpec.configure do |config|
  config.alias_example_to(:scenario)
  config.order = 'random'
end
