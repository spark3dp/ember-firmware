#  File: request_settings_command_steps.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Richard Greene
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
    RequestSettingsCommandSteps = RSpec::EM.async_steps do

      def assert_request_settings_command_handled_when_request_settings_command_received(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command

        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:data][:settings][:ProjectorLEDCurrent]).to eq(200)
          expect(request_params[:data][:settings][:InspectionHeightMicrons]).to eq(65000)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: REQUEST_SETTINGS_COMMAND,
          task_id: test_task_id,
        )
      end

      def assert_error_acknowledgement_sent_when_request_settings_command_fails(&callback)

        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(REQUEST_SETTINGS_COMMAND)
          expect(request_params[:job_id]).to eq(nil) # when no job_id included with command
          expect(request_params[:data][:message]).to match_log_message(
                  LogMessages::EXCEPTION_BRIEF,
                  Errno::ENOENT.new(smith_settings_file))

        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(
          command: REQUEST_SETTINGS_COMMAND,
          task_id: test_task_id,
        )
      end

    end
  end
end
