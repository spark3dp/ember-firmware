#  File: print_engine_command_steps.rb
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

module Smith
  module Client
    PrintEngineCommandSteps = RSpec::EM.async_steps do

      def assert_command_acknowledged_and_forwarded_to_command_pipe(command, &callback)
        d1 = add_command_pipe_expectation do |command|
          expect(command).to eq(command)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(command)
        end

        d3 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(command)
        end

        when_succeed(d1, d2, d3) { callback.call }

        dummy_server.post_command(command: command, task_id: test_task_id)
      end

      def assert_error_acknowledgement_sent_when_print_engine_command_fails(command, &callback)
        d1 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(command)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::FAILED_ACK)
          expect(request_params[:data][:command]).to eq(command)
          expect(request_params[:data][:message]).to match_log_message(
            LogMessages::EXCEPTION_BRIEF,
            Printer::CommunicationError.new('')
          )
        end

        when_succeed(d1, d2) { callback.call }

        dummy_server.post_command(command: command, task_id: test_task_id)
      end

      def assert_command_acknowledged_and_forwarded_to_command_pipe_when_message_is_a_hash(&callback)

        d1 = add_command_pipe_expectation do |command|
          expect(command).to eq(CMD_CANCEL)
        end

        d2 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::RECEIVED_ACK)
          expect(request_params[:data][:command]).to eq(CMD_CANCEL)
        end

        d3 = add_http_request_expectation acknowledge_endpoint(command_context) do |request_params|
          expect(request_params[:data][:state]).to eq(Command::COMPLETED_ACK)
          expect(request_params[:data][:command]).to eq(CMD_CANCEL)
        end

        when_succeed(d1, d2, d3) { callback.call }

        dummy_server.post_command(command: CMD_CANCEL, task_id: test_task_id, message_type: 'Hash')
      end

    end
  end
end
