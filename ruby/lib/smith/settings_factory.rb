#  File: settings_factory.rb
#  Factory to construct a settings object
#  See settings.rb for more information
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
  class SettingsError < StandardError; end
  module SettingsFactory
    module_function

    def build(settings)
      # Check for environment overrides
      settings.each do |key, value|
        if overridden_value = ENV["SMITH_#{key.to_s.upcase}"]
          settings[key] =
            if value.is_a?(String)
              overridden_value
            elsif value.is_a?(Fixnum)
              overridden_value.to_i
            else
              raise(SettingsError, "cannot convert overridden setting value for #{key.to_s.inspect} to #{value.class}")
            end
        end
      end

      settings_class = Struct.new(*settings.keys)
      settings_class.class_eval do
        def method_missing(method_name, *args, &block)
          raise(SettingsError, "setting #{method_name.to_s.inspect} does not exist")
        end
      end
      settings_class.new(*settings.values)
    end

  end
end
