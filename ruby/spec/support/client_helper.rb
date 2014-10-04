module ClientHelper
  require 'logger'

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      
      let(:client_command_pipe) { tmp_dir 'client_command_pipe' }
      let(:registration_info_file) { tmp_dir 'registration_info' }
      let(:print_settings_file) { tmp_dir 'printsettings' }

      steps = RSpec::EM.async_steps do
        def stop_client_async(&callback)
          EM.add_shutdown_hook { callback.call }
          @event_loop.stop
        end

        def set_settings_async(settings, &callback)
          settings.each do |key, value|
            ENV[key.to_s.upcase] = value.to_s
          end
          callback.call
        end
      end

      include steps
    end
  end

  def start_client
    ENV['CLIENT_COMMAND_PIPE'] = client_command_pipe
    ENV['REGISTRATION_INFO_FILE'] = registration_info_file
    ENV['PRINT_SETTINGS_FILE'] = print_settings_file
    
    # If watch_log_async is called before this method then @log_write_io
    # is used as the log device otherwise calls to the logger are no ops
    Smith::Client.enable_logging(@log_write_io)
    Smith::Client.logger.level = Logger::DEBUG

    Smith::Client.enable_faye_logging if $faye_log_enable
   
    @event_loop = Smith::Client::EventLoop.new(OpenStruct.new(auth_token: nil), retry_interval)
    @event_loop.start
  end

  def registration_file_contents
    JSON.parse(File.read(registration_info_file))
  end
  
  def print_settings_file_contents
    JSON.parse(File.read(print_settings_file))
  end

end
