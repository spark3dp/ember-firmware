require 'logger'
require 'json'

module ClientHelper

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      
      let(:client_command_pipe) { tmp_dir 'client_command_pipe' }
      let(:registration_info_file) { tmp_dir 'registration_info' }
      let(:print_settings_file) { tmp_dir 'printsettings' }
      let(:state) { Smith::State.load }

      steps = RSpec::EM.async_steps do
        def stop_client_async(&callback)
          # Clear the logger so any attempts to write to log IO after the the client is stopped are no-ops
          Smith::Client.logger = nil
          EM.add_shutdown_hook { callback.call }
          @event_loop.stop
        end

        def set_settings_async(settings, &callback)
          settings.each do |key, value|
            Smith::Settings.send("#{key}=", value)
          end
          callback.call
        end
      end

      include steps
    end
  end

  def start_client
    Smith::Settings.registration_info_file = registration_info_file
    Smith::Settings.print_settings_file = print_settings_file
    
    # If watch_log_async is called before this method then @log_write_io
    # is used as the log device otherwise calls to the logger are no ops
    Smith::Client.enable_logging(@log_write_io)
    Smith::Client.logger.level = Logger::DEBUG

    Smith::Client.enable_faye_logging if $faye_log_enable
   
    @event_loop = Smith::Client::EventLoop.new(state, retry_interval)
    @event_loop.start
  end

  def registration_file_contents
    JSON.parse(File.read(registration_info_file))
  end
  
  def print_settings_file_contents
    JSON.parse(File.read(print_settings_file))
  end

  def subscribe_to_test_channel(&block)
    step_method = caller[0].sub(Dir.getwd, '.')
    timer = EM.add_timer(2) { raise "Timeout waiting for test notification from dummy server (subscription added #{step_method})" }
    subscription = Faye::Client.new("#{dummy_server.url}/faye").subscribe('/test') do |raw_payload|
      block.call(JSON.parse(raw_payload, symbolize_names: true)) if block
      EM.cancel_timer(timer)
    end
    subscription.errback { raise "error subscribing to test channel (subscription added #{step_method}" }
    subscription
  end

end
