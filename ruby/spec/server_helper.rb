#  File: server_helper.rb
#  Spec helper for any Smith::Server tests
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
