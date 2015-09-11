#  File: command.rb
#  Sinatra routes for handling commands
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
  module Server

    # Commands supported by server that are handled directly instead of through smith
    CMD_GET_STATUS = 'GETSTATUS'
    CMD_GET_BOARD_NUM = 'GETBOARDNUM'
    CMD_GET_FW_VERSION = 'GETFWVERSION'

    class Application < Sinatra::Base

      helpers do
        def validate_command(command)
          return unless command.nil? || command.strip.empty?
          halt 400, { error: 'Command parameter is required' }.to_json
        end
      end

      post '/command' do
        content_type 'application/json'
        command = params[:command]
        validate_command(command)

        begin
          case command.upcase.strip
          when CMD_GET_STATUS
            { command: command, response: Printer.get_status }.to_json
          when CMD_GET_BOARD_NUM
            { command: command, response: Printer.serial_number }.to_json
          when CMD_GET_FW_VERSION
            { command: command, response: VERSION }.to_json
          else
            Printer.send_command(command)
            { command: command }.to_json
          end
        rescue Smith::Printer::CommunicationError => e
          halt 500, { error: e.message }.to_json
        end

      end

    end
  end
end
