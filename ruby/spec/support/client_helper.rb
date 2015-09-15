#  File: client_helper.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'logger'
require 'json'

module ClientHelper

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      
      let(:client_command_pipe) { tmp_dir 'client_command_pipe' }
      let(:registration_info_file) { tmp_dir 'registration_info' }
      let(:settings_file) { tmp_dir 'settings' }
      let(:status_pipe) { tmp_dir 'status_pipe' }
      let(:periodic_status_interval) { 15 }
      let(:test_task_id) { '123456' }
      let(:command_context) { Context.new(task_id: test_task_id) }
      let(:smith_settings_file) { tmp_dir 'smith_settings_file' }
      let(:test_printer_status_values) { { state: Smith::HOME_STATE, ui_sub_state: Smith::NO_SUBSTATE, spark_state: 'ready', error_code: 0, error_message: 'no error', spark_job_state: '' } }
      let(:test_printer_status) { printer_status(test_printer_status_values) }
      let(:test_status_payload) { status_payload(test_printer_status) }
      let(:test_serial_number) {'abcd1234'}

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

        # Update client settings with test specific values
        def set_test_specific_settings_async(&callback)
          Smith::Settings.registration_info_file = registration_info_file
          Smith::Settings.settings_file = settings_file
          Smith::Settings.status_pipe = status_pipe
          Smith::Settings.client_retry_interval = 0.01
          Smith::Settings.client_periodic_status_interval = periodic_status_interval
          Smith::Settings.smith_settings_file = smith_settings_file
          callback.call
        end

        # Update the client state
        # If a state parameter is not specified, the valid test auth_token is used
        # Client will skip primary registration if auth_token is known and valid
        def set_client_state_async(state_params = nil, &callback)
          @state.update(state_params || { auth_token: dummy_server.auth_token, printer_id: dummy_server.printer_id })
          callback.call
        end

        def set_smith_settings_async(settings, &callback)
          File.write(smith_settings_file, JSON.pretty_generate(Smith::SETTINGS_ROOT_KEY => settings))
          callback.call
        end

        def delete_smith_settings_async(&callback)
          File.delete(smith_settings_file)
          callback.call
        end
      end

      include steps
    end
  end

  def start_client
    # If watch_log_async is called before this method then @log_write_io
    # is used as the log device otherwise calls to the logger are no ops
    Smith::Client.enable_logging(Logger::DEBUG, @log_write_io)

    Smith::Client.enable_faye_logging if $faye_log_enable

    allow(Smith::Printer).to receive(:serial_number).and_return(test_serial_number)
 
    EM.add_shutdown_hook do
      # Change the log device so any logger calls will be redirected to standard
      # out or ignored if the event loop stops rather than going to a broken pipe
      if $client_log_enable
        Smith::Client.enable_logging(Logger::DEBUG, STDOUT)
      else
        Smith::Client.logger = nil
      end
    end

    @event_loop = Smith::Client::EventLoop.new(@state)
    @event_loop.start
  end

  def registration_file_contents
    JSON.parse(File.read(registration_info_file))
  end
  
  def test_settings_file_contents
    JSON.parse(File.read(settings_file))
  end

  # Call specified callback when specified deferrable objects succeed
  def when_succeed(*deferrables, &block)
    multi = EM::MultiRequest.new
    deferrables.each do |d|
      multi.add(d.inspect, d)
    end
    multi.callback do
      block.call
    end
  end

end
