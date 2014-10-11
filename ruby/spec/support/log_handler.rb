module LogHandler

  class LogSubscription
    def initialize(callback, connection)
      @callback, @connection = callback, connection
    end

    def call(*args)
      # Call the subscription callback with this object to callback can cancel subscription if desired
      args.push(self)
      @callback.call(*args)
    end

    def cancel
      @connection.cancel_subscription(self)
    end
  end

  def initialize(copy_to_stdout)
    # Treat expectations array as FIFO queue
    # expectations[0] is the rear of the queue
    # expectations[n] is the front of the queue
    @error_expectations = []
    @warn_expectations = []
    @subscriptions = []
    @copy_to_stdout = copy_to_stdout
  end

  def add_error_expectation(&callback)
    @error_expectations.unshift(callback)
  end

  def add_warn_expectation(&callback)
    @warn_expectations.unshift(callback)
  end

  def add_subscription(&callback)
    @subscriptions.push(LogSubscription.new(callback, self))
  end

  def cancel_subscription(subscription)
    @subscriptions.delete(subscription)
  end

  def callbacks_empty?
    @error_expectations.empty? && @warn_expectations.empty?
  end

  def receive_data(data)
    # Split data into array of log entries
    entries = data.split("\n")
    
    # Listeners just get called with whatever is read from the log io
    # Order is not important as it is with expectations
    @subscriptions.each do |l|
      l.call(entries)
    end
    
    # Each registered callback corresponds to an expected log entry
    # data may contain multiple entries
    entries.each do |entry|
      puts entry if @copy_to_stdout

      case entry[0]
      when 'E'
        if callback = @error_expectations.pop
          callback.call(entry)
        end
      when 'W'
        if callback = @warn_expectations.pop
          callback.call(entry)
        end
      end

    end
  end

end
