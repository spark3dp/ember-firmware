require 'smith/printer'

module PipeHandler

  def initialize
    # Treat expectations array as FIFO queue
    # expectations[0] is the rear of the queue
    # expectations[n] is the front of the queue
    @expectations = []
  end

  def add_expectation(&callback)
    @expectations.unshift(callback)
  end

  def notify_readable
    @io.readlines.map { |line| line.sub("\n", '') }.each do |command|
      if command != Smith::Printer::Commands::GET_STATUS
        # Each registered callback corresponds to an expected command in the command pipe
        call_expectation_callback(command)
      end
    end
  end

  private

  def call_expectation_callback(*args)
    callback = @expectations.pop
    callback.call(*args) if callback
  end

end
