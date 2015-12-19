require 'json'
require 'support/definitions'

# Provides access to main firmware settings file

module Tests
  class Settings

    def initialize(path)
      @settings = JSON.parse(File.read(path)).fetch(SETTINGS_ROOT_KEY)
    end

    def get(key)
      @settings.fetch(key)
    end

  end
end