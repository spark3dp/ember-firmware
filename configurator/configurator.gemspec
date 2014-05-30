# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'configurator/version'

Gem::Specification.new do |spec|
  spec.name          = 'configurator'
  spec.version       = Configurator::VERSION
  spec.authors       = ['Jason Lefley']
  spec.email         = ['jason.lefley@autodesk.com']
  spec.summary       = %q{Network configuration firmware}
  spec.homepage      = 'http://autodesk.com'
  spec.license       = 'Propritery'

  spec.files         = Dir['{lib}/**/*', 'Rakefile']
  spec.executables   = ['load_wifi_config']
  spec.test_files    = Dir['spec/**/*']
  spec.require_paths = ['lib']

  spec.add_development_dependency 'rake'
  spec.add_development_dependency 'bundler', '~> 1.6'
  spec.add_development_dependency 'rspec',   '~> 2.14.1'
  spec.add_development_dependency 'sshkit',  '~> 1.4.0'

  spec.add_dependency 'open4', '~> 1.3.4'
end
