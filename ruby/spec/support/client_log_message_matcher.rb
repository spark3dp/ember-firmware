RSpec::Matchers.define :match_log_message do |*expected|
  match do |actual|
    actual.match(format_message_regexp(*expected))
  end

  failure_message do |actual|
    "expected #{actual.inspect} to match #{format_message_regexp(*expected)}"
  end

  def format_message_regexp(*args)
    Regexp.quote(Smith::Client::LogMessage.format(*args))
  end
end
