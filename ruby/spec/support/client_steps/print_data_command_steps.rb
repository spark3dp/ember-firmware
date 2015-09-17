#  File: print_data_command_steps.rb
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

require 'fileutils'

module Smith
  module Client
    PrintDataCommandSteps = RSpec::EM.async_steps do

      def assert_print_data_command_handled_when_print_data_command_received_when_file_not_already_loaded_when_print_data_load_succeeds(&callback)
        d1 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_SHOW_PRINT_DATA_DOWNLOADING)
        end

        d2 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_START_PRINT_DATA_LOAD)
        end

        d3 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_PROCESS_PRINT_DATA)
          expect(File.read(File.join(print_data_dir, test_print_file))).to eq("test print file contents\n")
          expect(test_settings_file_contents).to eq(final_print_settings)
        end

        d4 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
        end

        d5 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
        end

        when_succeed(d1, d2, d3, d4, d5) { callback.call }

        dummy_server.post_command(
          command: PRINT_DATA_COMMAND,
          task_id: test_task_id,
          file_url: dummy_server.test_print_file_url,
          settings: print_settings,
          job_id: test_job_id
        )
      end

      def assert_print_data_command_handled_when_print_data_command_received_when_file_already_loaded_when_load_settings_succeeds(&callback)

        d1 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_START_PRINT_DATA_LOAD)
        end
        
        d2 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_APPLY_SETTINGS)
          expect(test_settings_file_contents).to eq(final_print_settings)
        end
        
        d3 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_SHOW_PRINT_DATA_LOADED)
        end

        d4 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
        end

        d5 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
        end

        when_succeed(d1, d2, d3, d4, d5) { callback.call }

        dummy_server.post_command(
          command: PRINT_DATA_COMMAND,
          task_id: test_task_id,
          file_url: "#{dummy_server.url}/#{print_file_name}",
          settings: print_settings,
          job_id: test_job_id
        )
      end

      def assert_print_data_command_handled_when_print_data_command_received_when_file_already_loaded_when_printer_not_in_valid_state(&callback)
        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
          expect(request_params[:data][:message]).to match_log_message(
            LogMessages::EXCEPTION_BRIEF,
            Printer::InvalidState.new('')
          )
        end
       
        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: PRINT_DATA_COMMAND,
          task_id: test_task_id,
          file_url: "#{dummy_server.url}/#{print_file_name}",
          settings: print_settings,
          job_id: test_job_id
        )
      end

      def assert_print_data_dir_purged_before_print_file_download(&callback)
        expect(File.exists?(stray_print_file)).to eq(false)
        callback.call
      end

      def touch_stray_print_file(&callback)
        # Create old print file that needs to be deleted before downloading the new file
        # A stray file might exist as a result of an error during print data processing
        FileUtils.touch(stray_print_file)
        callback.call
      end

      def assert_error_acknowledgement_sent_when_print_data_command_received_when_printer_not_in_valid_state_after_download(&callback)
        d1 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_SHOW_PRINT_DATA_DOWNLOADING)
          set_printer_status(state: CALIBRATING_STATE, spark_state: 'maintenance', error_code: 0, error_message: 'no error', spark_job_state: '')
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
        end

        d3 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(test_job_id)
          expect(request_params[:data][:message]).to match_log_message(
            LogMessages::EXCEPTION_BRIEF,
            Printer::InvalidState.new('')
          )
        end
       
        when_succeed(d1, d2, d3) { callback.call }

        dummy_server.post_command(
          command: PRINT_DATA_COMMAND,
          task_id: test_task_id,
          file_url: dummy_server.test_print_file_url,
          settings: print_settings,
          job_id: test_job_id
        )
      end

      def assert_error_acknowledgement_sent_when_print_data_command_received(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:data][:message]).to match_log_message(
            LogMessages::EXCEPTION_BRIEF,
            Printer::InvalidState.new('')
          )
        end
       
        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: PRINT_DATA_COMMAND,
          task_id: test_task_id,
          file_url: dummy_server.test_print_file_url,
          settings: print_settings,
          job_id: test_job_id
        )
      end

      def assert_print_file_not_downloaded(&callback)
        expect(File.exist?(File.join(print_data_dir, test_print_file))).to be(false)
        callback.call
      end

      def assert_error_acknowledgement_sent_when_print_data_command_received_when_download_fails(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(PRINT_DATA_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
          expect(request_params[:data][:message]).to match_log_message(
            LogMessages::PRINT_DATA_DOWNLOAD_ERROR,
            dummy_server.invalid_url
          )
        end

        d3 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_SHOW_PRINT_DATA_DOWNLOADING)
        end

        d4 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_SHOW_PRINT_DOWNLOAD_FAILED)
        end

        when_succeed(d1, d2, d3, d4) { callback.call }

        dummy_server.post_command(
          command: PRINT_DATA_COMMAND,
          task_id: test_task_id,
          file_url: dummy_server.invalid_url,
          settings: print_settings
        )
      end

    end
  end
end
