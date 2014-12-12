module LogHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      require 'fcntl'

      steps = RSpec::EM.async_steps do

        def watch_log_async(copy_to_stdout, &callback)
          @log_read_io, @log_write_io = IO.pipe
          @log_connection = EM.attach(@log_read_io, LogConnection, copy_to_stdout)
          callback.call
        end

        def stop_watching_log_async(&callback)
          @log_connection.detach if @log_connection
          @log_read_io.close if @log_read_io
          @log_write_io.close if @log_write_io
          callback.call
        end

      end

      include steps
    end
  end

  # Takes the arguments to the format log message method and calls block when a log entry matching the generated string is logged
  # Returns a deferrable for convenience when setting multiple callbacks in a single step
  def add_log_subscription(*args, &block)
    filter = Smith::Client::LogMessage.format(*args)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for log entry matching #{filter.inspect} (subscription added #{step_method})" }
    deferrable = EM::DefaultDeferrable.new
    subscription = @log_connection.add_subscription do |entries|
      match = entries.select { |e| e.match(Regexp.quote(filter)) }.first
      if match
        EM.cancel_timer(timer)
        block.call if block
        deferrable.succeed
      end
    end
    deferrable
  end

end
