#\ -p 4567
ENV['RACK_ENV'] ||= 'development'
root = File.expand_path('../../..', __FILE__)
$:.unshift(root)
require File.join(root, 'smith/server/application.rb')
use Rack::CommonLogger
use Rack::ShowExceptions
run Smith::Server::Application
