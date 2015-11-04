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
     
      let(:response_body) { JSON.parse(last_response.body, symbolize_names: true) }
      let(:test_settings) { { SETTINGS_ROOT_KEY => {ProjectorLEDCurrent:-1}  } }
      let(:good_settings) { { SETTINGS_ROOT_KEY => {ProjectorLEDCurrent:123} } }
      let(:bad_settings)  { { SETTINGS_ROOT_KEY => {ProjectorLEDCurrent:1.0} } }
      let(:settings_file) { tmp_dir 'settings' }
      let(:smith_settings_file) { tmp_dir 'smith_settings_file' }

      def initialize_settings
        Smith::Settings.settings_file = settings_file
        Smith::Settings.smith_settings_file = smith_settings_file
        File.write(smith_settings_file, JSON.pretty_generate(test_settings))
      end

      context 'when communication via command pipe is possible' do

        before do
          create_command_pipe
          open_command_pipe
          initialize_settings
        end

        after do
          close_command_pipe
          File.delete(smith_settings_file)
        end

        scenario 'accepts valid settings' do
          put '/settings', settings: good_settings.to_json

          expect(last_response.status).to eq(200)
          expect(Printer.settings['ProjectorLEDCurrent']).to eq(123)
        end

        scenario 'does not accept invalid settings' do
          put '/settings', settings: bad_settings.to_json

          expect(last_response.status).to eq(500)
          expect(response_body[:error]).to match(/Unable to set settings/i)
          expect(Printer.settings['ProjectorLEDCurrent']).to eq(-1)
        end

      end

      context 'when communication via command pipe is not possible' do

        before do
          initialize_settings
        end

        after do
          File.delete(smith_settings_file)
        end

        scenario 'does not accept settings' do
          put '/settings', settings: good_settings.to_json
          
          expect(last_response.status).to eq(500)
          expect(response_body[:error]).to match(/Unable to set settings/i)
          expect(Printer.settings['ProjectorLEDCurrent']).to eq(-1)
        end

        scenario 'gets settings' do
          get '/settings'

          expect(last_response.status).to eq(200)
          expect(response_body[:response]).to eq(test_settings.to_json)
        end

      end

    end
  end
end
