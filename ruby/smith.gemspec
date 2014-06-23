# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'smith/version'

Gem::Specification.new do |spec|
  spec.name          = 'smith'
  spec.version       = Smith::VERSION
  spec.authors       = ['Jason Lefley']
  spec.email         = ['jason.lefley@autodesk.com']
  spec.summary       = %q{Ruby component of Spark 3D printer firmware}
  spec.homepage      = 'http://autodesk.com'
  spec.license       = 'Propritery'

  spec.files         = Dir['{lib}/**/*', 'Rakefile']
  spec.executables   = ['smith-config', 'smith-server']
  spec.test_files    = Dir['spec/**/*']
  spec.require_paths = ['lib']

  spec.add_development_dependency 'rake'
  spec.add_development_dependency 'bundler',  '~> 1.6'
  spec.add_development_dependency 'rspec',    '~> 3.0.0'
  spec.add_development_dependency 'sshkit',   '~> 1.4.0'
  spec.add_development_dependency 'capybara', '~> 2.2.1'

  spec.add_dependency 'open4',           '~> 1.3.4'
  spec.add_dependency 'sinatra',         '~> 1.4.5'
  spec.add_dependency 'sinatra-partial', '~> 0.4.0'
  spec.add_dependency 'sinatra-contrib', '~> 1.4.2'
  spec.add_dependency 'thin',            '~> 1.6.2'
  spec.add_dependency 'thor',            '~> 0.19.1'
  spec.add_dependency 'ipaddress',       '~> 0.8.0'
end
