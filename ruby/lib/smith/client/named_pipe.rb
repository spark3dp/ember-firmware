# Class to encapsulate named pipe

require 'mkfifo'
require 'fcntl'

module Smith
  module Client
    class NamedPipe

      # Convenience methods for construction
      class << self
        def status_pipe
          new(Settings.status_pipe)
        end
      end

      def initialize(path)
        # path is the path to the named pipe
        @path = path

        # Ensure a named pipe located at path exists
        create
      end

      def create
        # Don't do anything if a named pipe already exists at path
        return if File.pipe?(@path)

        # If any filesystem node exists at path, delete it
        File.delete(@path) if File.exist?(@path)

        # Create the named pipe at path
        File.mkfifo(@path)
      end

      def io
        @io ||= IO.new(IO.sysopen(Settings.status_pipe, Fcntl::O_RDONLY | Fcntl::O_NONBLOCK))
      end

    end
  end
end
