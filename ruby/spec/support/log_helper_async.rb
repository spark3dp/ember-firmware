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

        def stop_watching_log_async(&callback)
          fail 'all log expectation callbacks not called' unless @log_connection.callbacks_empty?
          @log_connection.detach if @log_connection
          @log_read_io.close if @log_read_io
          @log_write_io.close if @log_write_io
          callback.call
        end

      end

      include steps
    end
  end

  def add_error_log_expectation(&block)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for write to log (expectation added #{step_method})" }
    @log_connection.add_error_expectation do |*args|
      block.call(*args) if block
      EM.cancel_timer(timer)
    end
  end

  def add_warn_log_expectation(&block)
    step_method = caller[0].sub(Dir.pwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for write to log (expectation added #{step_method})" }
    @log_connection.add_warn_expectation do |*args|
      block.call(*args) if block
      EM.cancel_timer(timer)
    end
  end

  # Takes a filter in the form of a regex/string and calls block when a log entry matching the filter is logged
  # Returns a deferrable for convenience when setting multiple callbacks in a single step
  def add_log_subscription(filter, &block)
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
