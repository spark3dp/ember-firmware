# Class to persist state
# This implementation is file backed

require 'ostruct'
require 'json'

module Smith
  class State < OpenStruct

    class << self

      def load
        new(JSON.parse(File.read(Settings.state_file), symbolize_name: true))
      rescue Errno::ENOENT
        new
      end

    end

    def save
      File.write(Settings.state_file, marshal_dump.to_json)
    end

    def update(hash)
      hash.each do |key, value|
        send("#{key}=", value)
      end
      save
    end

    def get_binding
      binding
    end

  end
end
