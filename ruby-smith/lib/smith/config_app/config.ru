$:.unshift(File.expand_path('../lib', __FILE__))
require 'smith/config_app/app'
run Smith::ConfigApp::App
