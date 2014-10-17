module Smith
  class NonblockingIO

    def initialize(io)
      @io = io
    end

    # Similiar to IO#readlines but will return empty array
    # rather than blocking if there is no data to read or
    # the EOF is not reached
    def readlines_nonblock
      lines = []
      buffer = ''
      loop do
        buffer << @io.read_nonblock(1)
        if buffer[-1] == "\n"
          lines << buffer
          buffer = ''
        end
      end
    rescue Errno::EAGAIN, EOFError
      return lines
    end

  end
end
