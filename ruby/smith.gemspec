# coding: utf-8

#  File: smith.gemspec
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

lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'smith/version'

Gem::Specification.new do |spec|
  spec.name          = 'smith'
  spec.version       = Smith::VERSION
  spec.authors       = ['Jason Lefley', 'Richard Greene']
  spec.email         = ['jason.lefley@autodesk.com', 'richard.greene@autodesk.com']
  spec.summary       = %q{Ruby component of Ember 3D printer firmware}
  spec.homepage      = 'http://autodesk.com'
  spec.license       = 'GPLv3 or later'

  spec.files         = Dir['{lib}/**/*', 'Rakefile']
  spec.executables   = ['smith-config', 'smith-server', 'smith-client']
  spec.test_files    = Dir['spec/**/*']
  spec.require_paths = ['lib']

  spec.add_development_dependency 'rake'
  spec.add_development_dependency 'bundler',            '~> 1.7'
  spec.add_development_dependency 'rspec',              '~> 3.1.0'
  spec.add_development_dependency 'sshkit',             '~> 1.4.0'
  spec.add_development_dependency 'capybara',           '~> 2.2.1'
  spec.add_development_dependency 'simplecov',          '~> 0.7.1'
  spec.add_development_dependency 'rails',              '~> 4.2.1'
  spec.add_development_dependency 'faye-rails',         '~> 2.0.0'
  spec.add_development_dependency 'httpclient',         '~> 2.4.0'
  spec.add_development_dependency 'rspec-eventmachine', '~> 0.2.0'
  spec.add_development_dependency 'webmock',            '~> 1.19.0'

  spec.add_dependency 'open4',           '~> 1.3.4'
  spec.add_dependency 'sinatra',         '~> 1.4.5'
  spec.add_dependency 'sinatra-partial', '~> 0.4.0'
  spec.add_dependency 'sinatra-contrib', '~> 1.4.2'
  spec.add_dependency 'thin',            '~> 1.6.3'
  spec.add_dependency 'thor',            '~> 0.19.1'
  spec.add_dependency 'ipaddress',       '~> 0.8.0'
  spec.add_dependency 'rack-flash3',     '~> 1.0.5'
  spec.add_dependency 'faye',            '~> 1.0.1'
  spec.add_dependency 'em-http-request', '~> 1.1.2'
  spec.add_dependency 'mkfifo',          '~> 0.0.1'
end
