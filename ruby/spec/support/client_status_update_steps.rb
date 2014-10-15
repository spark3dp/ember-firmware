ClientStatusUpdateSteps = RSpec::EM.async_steps do
 
  def assert_status_update_request_made_when_status_written_to_status_pipe(&callback)
    subscription = subscribe_to_test_channel do |payload|
      expect(payload[:request_endpoint]).to match(/printers\/539\/status/)
      expect(payload[:request_params].to_json).to eq(status_json)
      subscription.cancel
      callback.call
    end

    subscription.callback do
      # Simulate status update from smith
      File.write(status_pipe, status_json)
    end
  end

end

