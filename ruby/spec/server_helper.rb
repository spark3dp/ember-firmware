# This is the spec helper for any Smith::Server tests

ENV['RACK_ENV'] = 'test'

require 'common_helper'
require 'smith/server/application'

require 'capybara'
require 'capybara/dsl'

Capybara.app = Smith::Server::Application.new
Capybara.default_host = Capybara.app.settings.canonical_host

RSpec.configure do |config|
  config.include Capybara::DSL
end
