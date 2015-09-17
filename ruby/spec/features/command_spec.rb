#  File: command_spec.rb
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

require 'server_helper'

module Smith
  module Server
    describe 'command interface', :tmp_dir do
      include PrintEngineHelper
      include Rack::Test::Methods
     
      let(:response_body) { JSON.parse(last_response.body, symbolize_names: true) }

      context 'when communication via command pipe is possible' do

        before do
          create_command_pipe
          create_printer_status_file
          open_command_pipe
        end

        after do
          close_command_pipe
        end

        scenario 'receives command that does not return a response' do
          post '/command', command: CMD_CANCEL

          expect(next_command_in_command_pipe).to eq(CMD_CANCEL)
          expect(last_response.status).to eq(200)
          expect(response_body).to eq(command: CMD_CANCEL)
        end

        scenario 'receives a request without command parameter' do
          post '/command'

          expect(last_response.status).to eq(400)
          expect(response_body[:error]).to match(/command parameter is required/i)
        end

        scenario 'receives get status command' do
          status = { state: HOME_STATE, ui_sub_state: NO_SUBSTATE }
          set_printer_status(status)

          post '/command', command: CMD_GET_STATUS

          expect(last_response.status).to eq(200)
          expect(response_body[:command]).to eq(CMD_GET_STATUS)
          expect(response_body[:response]).to eq(keys_to_symbols(printer_status(status)))
        end

      end

      context 'when communication via command pipe is not possible' do

        scenario 'receives a command' do
          post '/command', command: CMD_CANCEL
          
          expect(last_response.status).to eq(500)
          expect(response_body[:error]).to match(/Unable to send command/i)
        end

      end

      scenario 'receives get board serial number command' do
        allow(Printer).to receive(:serial_number).and_return('abc123')
        post '/command', command: CMD_GET_BOARD_NUM

        expect(last_response.status).to eq(200)
        expect(response_body[:command]).to eq(CMD_GET_BOARD_NUM)
        expect(response_body[:response]).to eq('abc123')
      end

      scenario 'receives get firmware version command' do
        post '/command', command: CMD_GET_FW_VERSION

        expect(last_response.status).to eq(200)
        expect(response_body[:command]).to eq(CMD_GET_FW_VERSION)
        expect(response_body[:response]).to eq(VERSION)
      end

    end
  end
end
