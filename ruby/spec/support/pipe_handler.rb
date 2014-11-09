require 'smith/printer'

module PipeHandler

  # PipeHandler is notified when the command pipe is readable and reads commands
  # from the pipe.  Each line on the command pipe is a command and requires a corresponding
  # callback.  The callback is called with the command from the pipe and if a corresponding
  # callback does not exist, an error is raised.

  def initialize
    # Treat array as FIFO queue
    # array[0] is the rear of the queue
    # array[n] is the front of the queue
    @callbacks = []
  end

  # Add a callback to be called when a command is read
  # from the pipe to the queue of callbacks 
  def add_callback(&callback)
    @callbacks.unshift(callback)
  end

  def notify_readable
    @io.readlines.map { |line| line.sub("\n", '') }.each do |command|
      # Each registered callback corresponds to an expected command in the command pipe
      call_next_callback(command) if command != Smith::CMD_GET_STATUS
    end
  end

  def callbacks_empty?
    @callbacks.empty?
  end

  private

  def call_next_callback(*args)
    if callback = @callbacks.pop
      callback.call(*args)
    else
      fail "command pipe received unexpected command: #{args.first.inspect}"
    end
  end

end
