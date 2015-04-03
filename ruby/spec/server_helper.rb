# This is the spec helper for any Smith::Server tests

ENV['RACK_ENV'] = 'test'

require 'rack/test'

require 'common_helper'
require 'smith/server/application'

require 'capybara'
require 'capybara/dsl'

Capybara.app = Smith::Server::Application.new

mixin = Module.new do
  def app
    Capybara.app
  end
end

RSpec.configure do |config|
  config.include Capybara::DSL
  config.include mixin
end
