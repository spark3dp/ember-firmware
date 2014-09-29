ENV['RACK_ENV'] ||= 'development'
root = File.expand_path('../../..', __FILE__)
$:.unshift(root)
require File.join(root, 'smith/server/application.rb')
run Smith::Server::Application
