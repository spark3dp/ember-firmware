require 'smith/printer'

module PipeConnection

  # Connection to named pipe that is notified when the command pipe is readable and reads commands
  # from the pipe.  Each line on the command pipe is a command and requires a corresponding
  # expectation or allowance.  Expectations take a block that is called when a command is received
  # and enforce order while allowances allow a command to be received.  If an expectation or
  # allowance does not exist for a received command, an error is raised.

  def initialize
    # Treat arrays as FIFO queue
    # array[0] is the rear of the queue
    # array[n] is the front of the queue
    @expectation_callbacks = []
    @allowed_commands = []
  end

  def add_expectation(&callback)
    @expectation_callbacks.unshift(callback)
  end

  def allow_command(command)
    @allowed_commands.unshift(command)
  end

  def notify_readable
    @io.readlines.map { |line| line.sub("\n", '') }.each do |command|
      if @allowed_commands.last == command
        # This command is allowed, remove the allowance now that it has been received
        @allowed_commands.pop
      else
        # Otherwise an expectation must be present
        call_next_expectation_callback(command)
      end
    end
  end

  def all_expectations_met?
    @expectation_callbacks.empty?
  end

  private

  def call_next_expectation_callback(*args)
    if callback = @expectation_callbacks.pop
      callback.call(*args)
    else
      fail "command pipe received unexpected command: #{args.first.inspect}"
    end
  end

end
