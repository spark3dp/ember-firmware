module LogHandler

  def initialize(copy_to_stdout)
    # Treat expectations array as FIFO queue
    # expectations[0] is the rear of the queue
    # expectations[n] is the front of the queue
    @error_expectations = []
    @warn_expectations = []
    @copy_to_stdout = copy_to_stdout
  end

  def add_error_expectation(&callback)
    @error_expectations.unshift(callback)
  end

  def add_warn_expectation(&callback)
    @warn_expectations.unshift(callback)
  end

  def receive_data(data)
    # Each registered callback corresponds to an expected log entry
    # data may contain multiple entries
    data.split("\n").each do |entry|
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
