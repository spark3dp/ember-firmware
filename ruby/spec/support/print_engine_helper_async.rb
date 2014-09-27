module PrintEngineHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      require 'fcntl'
      include PrintEngineHelper

      steps = RSpec::EM.async_steps do

        def create_command_pipe_async(&callback)
          create_command_pipe
          callback.call
        end

        def create_command_response_pipe_async(&callback)
          create_command_response_pipe
          callback.call
        end

        def watch_command_pipe_async(&callback)
          fd = IO.sysopen(command_pipe, Fcntl::O_RDONLY | Fcntl::O_NONBLOCK)
          @command_pipe_io = IO.new(fd, Fcntl::O_RDONLY | Fcntl::O_NONBLOCK)
          @command_pipe_connection = EM.watch(@command_pipe_io, PipeHandler)
          @command_pipe_connection.notify_readable = true
          callback.call
        end

        def close_command_pipe_async(&callback)
          @command_pipe_io.close if @command_pipe_io
          callback.call
        end

        def open_command_response_pipe_async(&callback)
          open_command_response_pipe
          callback.call
        end

        def close_command_response_pipe_async(&callback)
          close_command_response_pipe
          callback.call
        end

        def write_get_status_command_response_async(status, &callback)
          write_get_status_command_response(status)
          callback.call
        end

      end

      include steps
    end
  end

  def add_command_pipe_expectation(callback, &block)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for write to command pipe (expectation added #{step_method})" }
    @command_pipe_connection.add_expectation do |*args|
      block.call(*args)
      callback.call
      EM.cancel_timer(timer)
    end
  end

end
