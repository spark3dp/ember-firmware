module DummyServerHelper

  def dummy_server
    $dummy_server
  end

  def subscribe_to_test_channel(&block)
    step_method = caller[0].sub(Dir.getwd, '.')
    notified = EM::DefaultDeferrable.new
    subscription = nil
    timer = EM.add_timer(2) do
      subscription.cancel
      # Let the cancellation go through 
      EM.next_tick { raise "Timeout waiting for test notification from dummy server (subscription added #{step_method})" }
    end
    subscription = Faye::Client.new("#{dummy_server.url}/faye").subscribe('/test') do |raw_payload|
      block.call(JSON.parse(raw_payload, symbolize_names: true)) if block
      EM.cancel_timer(timer)
    end
    subscription.errback { raise "error subscribing to test channel (subscription added #{step_method}" }
    subscription
  end

end
