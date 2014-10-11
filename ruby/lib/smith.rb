require 'smith/version'
require 'smith/definitions'
require 'smith/settings'
require 'smith/state'

module Smith
  module_function

  def root
    @root ||= File.expand_path('../..', __FILE__)
  end
end

