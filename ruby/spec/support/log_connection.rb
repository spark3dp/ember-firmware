module LogConnection

  class LogSubscription
    def initialize(callback, connection)
      @callback, @connection = callback, connection
    end

    def call(entries)
      @callback.call(entries)
    end

    def cancel
      @connection.cancel_subscription(self)
    end
  end

  def initialize(copy_to_stdout)
    @subscriptions = []
    @copy_to_stdout = copy_to_stdout
    @entries = []
  end

  def add_subscription(&callback)
    subscription = LogSubscription.new(callback, self)
    @subscriptions.push(subscription)
    subscription
  end

  def cancel_subscription(subscription)
    @subscriptions.delete(subscription).inspect
  end

  def entries
    @entries
  end

  def receive_data(data)
    # Split data into array of log entries
    entries = data.split("\n")

    # Store the entries logged
    @entries.push(*entries)

    # Print log entries if enabled
    puts entries if @copy_to_stdout

    # Listeners just get called with whatever is read from the log io
    # Order is not important as it is with expectations
    @subscriptions.each { |l| l.call(entries) }
  end

end
