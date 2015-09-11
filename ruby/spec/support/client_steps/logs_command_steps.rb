#  File: logs_command_steps.rb
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

require 'zlib'
require 'rubygems/package'

module Smith
  module Client
    LogsCommandSteps = RSpec::EM.async_steps do

      def assert_logs_command_handled_when_logs_command_received(&callback)
        # Subscribe to the test channel to receive notification of client posting command acknowledgement
        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
        end

        # Expect the client to have uploaded the log archive
        d3 = add_http_request_expectation dummy_server.test_upload_url do |request_params|
          # Extract the archive and verify the contents
          tar_reader = Gem::Package::TarReader.new(Zlib::GzipReader.new(StringIO.new(Base64.decode64(request_params[:body]))))
          tar_reader.rewind
          tar_reader.each do |entry|
            # If everything worked, this is the contents of the file that was written below!
            expect(entry.read).to eq('log file contents')
          end
          tar_reader.close
        end

        when_succeed(d1, d2, d3) { callback.call }

        # Create a sample log file
        File.write(tmp_dir('syslog'), 'log file contents')

        dummy_server.post_command(command: LOGS_COMMAND, task_id: test_task_id, upload_url: dummy_server.test_upload_url)
      end

      def assert_error_acknowledgement_sent_when_log_command_fails_due_to_http_error_received(&callback)
        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
        end
        
        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
          expect(request_params[:data][:message]).to match_log_message(LogMessages::LOG_UPLOAD_HTTP_ERROR, dummy_server.invalid_url, 404)
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(command: LOGS_COMMAND, task_id: test_task_id, upload_url: dummy_server.invalid_url)
      end

      def assert_error_acknowledgement_sent_when_log_command_fails_due_to_unreachable_server(&callback)
        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
        end
        
        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(LOGS_COMMAND)
          expect(request_params[:data][:message]).to match_log_message(LogMessages::LOG_UPLOAD_URL_UNREACHABLE, 'http://nothing')
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(command: LOGS_COMMAND, task_id: test_task_id, upload_url: 'http://nothing')
      end

    end
  end
end
