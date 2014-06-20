require 'bundler/setup'
require 'smith/version'
require 'smith/config'

module Smith
  module_function

  def root
    @root ||= File.expand_path('../..', __FILE__)
  end
end

Dir["#{Smith.root}/lib/smith/config/**/*.rb"].each { |f| require(f) }
