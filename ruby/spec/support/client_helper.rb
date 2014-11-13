require 'logger'
require 'json'

module ClientHelper

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      
      let(:client_command_pipe) { tmp_dir 'client_command_pipe' }
      let(:registration_info_file) { tmp_dir 'registration_info' }
      let(:print_settings_file) { tmp_dir 'printsettings' }
      let(:status_pipe) { tmp_dir 'status_pipe' }
      let(:health_check_interval) { 15 }
      let(:smith_settings_file) { tmp_dir 'smith_settings_file' }

      steps = RSpec::EM.async_steps do
        def stop_client_async(&callback)
          EM.add_shutdown_hook { callback.call }
          @event_loop.stop if @event_loop
        end

        def set_settings_async(settings, &callback)
          settings.each do |key, value|
            Smith::Settings.send("#{key}=", value)
          end
          callback.call
        end

        # Update the client state
        # If a state parameter is not specified, the valid test auth_token is used
        # Client will skip primary registration if auth_token is known and valid
        def set_client_state_async(state = nil, &callback)
          load_state
          @state.update(state || { auth_token: 'authtoken', printer_id: 539 })
          callback.call
        end

        def set_smith_settings_async(settings, &callback)
          File.write(smith_settings_file, JSON.pretty_generate(Smith::SETTINGS_ROOT_KEY => settings))
          callback.call
        end
      end

      include steps
    end
  end

  def start_client
    Smith::Settings.registration_info_file = registration_info_file
    Smith::Settings.print_settings_file = print_settings_file
    Smith::Settings.status_pipe = status_pipe
    Smith::Settings.client_retry_interval = 0.01
    Smith::Settings.client_health_check_interval = health_check_interval
    Smith::Settings.smith_settings_file = smith_settings_file
    
    # If watch_log_async is called before this method then @log_write_io
    # is used as the log device otherwise calls to the logger are no ops
    Smith::Client.enable_logging(Logger::DEBUG, @log_write_io)

    Smith::Client.enable_faye_logging if $faye_log_enable
 
    EM.add_shutdown_hook do
      # Change the log device so any logger calls will be redirected to standard
      # out or ignored if the event loop stops rather than going to a broken pipe
      if $client_log_enable
        Smith::Client.enable_logging(Logger::DEBUG, STDOUT)
      else
        Smith::Client.logger = nil
      end
    end

    @event_loop = Smith::Client::EventLoop.new
    @event_loop.start
  end

  def registration_file_contents
    JSON.parse(File.read(registration_info_file))
  end
  
  def print_settings_file_contents
    JSON.parse(File.read(print_settings_file))
  end

  # Create and return specified number of deferrable objects
  # Call the specified block when all deferrable objects receive #succeed
  def multi_deferrable(count, &block)
    multi = EM::MultiRequest.new
    deferrables = []
    count.times do |i|
      d = EM::DefaultDeferrable.new
      deferrables.push(d)
      multi.add(i.to_s, d)
    end
    multi.callback do
      block.call
    end
    deferrables
  end

  def load_state
    @state = Smith::State.load
  end

  # Convenience method for making assertions on urls
  # actual is the portion of a url after the hostname/port
  # expected_endpoint_helper_name is the name of the helper method in URLHelper that
  # returns the string that actual is expected to match
  def assert_endpoint_match(actual, expected_endpoint_helper_name)
    load_state
    expect(dummy_server.url + actual).to eq(send(expected_endpoint_helper_name))
  end

end
