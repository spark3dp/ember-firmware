#  File: settings_factory_spec.rb
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

require 'common_helper'
require 'smith/settings_factory'

module Smith
  describe SettingsFactory do

    context 'when building settings object' do

      describe 'returned settings object' do
        subject(:settings) { SettingsFactory.build(string_setting: 'value1', int_setting: 123) }

        it 'has getter and setter methods for the specified setting names and values' do
          expect(settings.string_setting).to eq('value1')

          settings.string_setting = 'new value'
          expect(settings.string_setting).to eq('new value')
        end


        context 'when attempting to get value for non-existent setting' do
          it 'raises error' do
            expect { settings.other }.to raise_error(SettingsError)
          end
        end

        context 'when attempting to set value for non-existent setting' do
          it 'raises error' do
            expect { settings.other = '123' }.to raise_error(SettingsError)
          end
        end

        context 'when string setting is overridden in environment' do

          before { ENV['SMITH_STRING_SETTING'] = 'overridden value' }

          it 'has overridden value' do
            expect(settings.string_setting).to eq('overridden value')
          end

        end
        
        context 'when integer setting is overridden in environment' do

          before { ENV['SMITH_INT_SETTING'] = '123' }

          it 'has overridden value having type int' do
            expect(settings.int_setting).to eq(123)
          end

        end

      end

      context 'when setting having default value of unsupported type is overridden in environment' do

        before { ENV['SMITH_SETTING'] = 'string' }
        
        it 'raises error' do
          expect { SettingsFactory.build(setting: Hash.new) }.to raise_error(SettingsError)
        end

      end

    end

  end
end
