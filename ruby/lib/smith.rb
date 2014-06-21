require 'bundler/setup'
require 'smith/version'

module Smith
  module_function

  def root
    @root ||= File.expand_path('../..', __FILE__)
  end
end

