#  File: settings_spec.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
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

require 'server_helper'

module Smith
  module Server
    describe 'settings interface', :tmp_dir do
      include PrintEngineHelper
      include Rack::Test::Methods
     
      let(:parsed_response_body) { JSON.parse(last_response.body, symbolize_names: true) }
      let(:settings) { { SETTINGS_ROOT_KEY => { 'ProjectorLEDCurrent' => -1  } } }
      let(:temp_settings_file) { tmp_dir 'temp_settings' }
      let(:smith_settings_file) { tmp_dir 'smith_settings' }

      scenario 'get settings successfully' do
        Smith::Settings.smith_settings_file = smith_settings_file
        File.write(smith_settings_file, JSON.pretty_generate(settings))

        get '/settings'

        expect(last_response.status).to eq(200)
        expect(JSON.parse(last_response.body)).to eq(settings)
      end

      context 'setting settings' do
        before { Smith::Settings.settings_file = temp_settings_file }
        
        scenario 'body is not valid JSON' do
          put '/settings', 'xxx'
        
          expect(last_response.status).to eq(400)
    #      expect(parsed_response_body[:error]).to match(/Unable to parse body as JSON/i)
          expect(parsed_response_body[:error]).to include('unexpected token at')
        end

        context 'when communication via command pipe is possible' do
          before do
            create_command_pipe
            open_command_pipe
          end

          after { close_command_pipe }

          scenario 'update settings successfully' do
            put '/settings', settings.to_json

            expect(last_response.status).to eq(200)

            # writes temp settings file
            expect(File.file?(temp_settings_file)).to eq(true)

            # writes specified settings to temp settings file
            expect(JSON.parse(File.read(temp_settings_file))).to eq(settings)

            # sends command to smith to apply settings written to temp settings file
            expect(next_command_in_command_pipe).to eq(CMD_APPLY_SETTINGS)
          end
        end

        context 'when communication via command pipe is not possible' do
          scenario 'fail to update settings' do
            put '/settings', settings.to_json
            
            expect(last_response.status).to eq(500)

            expect(parsed_response_body[:error]).to match(/Unable to send command/i)

            # does not write temp settings file
            # RG - doesn't it?
      #      expect(File.file?(temp_settings_file)).to eq(false)
          end
        end
      end

    end
  end
end
