ENV['RACK_ENV'] ||= 'development'
root = File.expand_path('../../..', __FILE__)
$:.unshift(root)
require File.join(root, 'smith/config_app/app.rb')
run Smith::ConfigApp::App
