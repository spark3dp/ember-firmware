#\ -p 4567
ENV['RACK_ENV'] ||= 'development'
root = File.expand_path('../../..', __FILE__)
$:.unshift(root)
require File.join(root, 'smith/app/application.rb')
run Smith::App::Application
