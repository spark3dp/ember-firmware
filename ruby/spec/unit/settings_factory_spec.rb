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
