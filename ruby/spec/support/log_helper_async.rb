module LogHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      require 'fcntl'

      steps = RSpec::EM.async_steps do

        def watch_log_async(copy_to_stdout, &callback)
          @log_read_io, @log_write_io = IO.pipe
          @log_connection = EM.attach(@log_read_io, LogHandler, copy_to_stdout)
          callback.call
        end

      end

      include steps
    end
  end

  class Timeout
    include EventMachine::Deferrable

    def start
      start_time = Time.now
      loop do
        sleep 0.05
        if (Time.now - start_time) > 2
          break
          fail
        end
      end
    end

  end

  def add_error_log_expectation(callback, &block)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for write to log (expectation added #{step_method})" }
    @log_connection.add_error_expectation do |*args|
      block.call(*args)
      callback.call
      EM.cancel_timer(timer)
    end
  end

  def add_warn_log_expectation(callback, &block)
    step_method = caller[0].sub(Dir.pwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for write to log (expectation added #{step_method})" }
    @log_connection.add_warn_expectation do |*args|
      block.call(*args)
      callback.call
      EM.cancel_timer(timer)
    end
  end

end
