#  File: edit.rb
#  Sinatra routes for wireless network configuration
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Cappie Pomeroy
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

require 'bigdecimal'

module Smith
  module Server
    class Application < Sinatra::Base

      get '/edit' do
        @all_settings = Printer.settings.to_json
        erb :edit
      end

      get '/edit/change_value' do
        @setting_to_change = params['setting']
        @change_value = params['value']
        current_settings = Printer.settings
        @new_settings = current_settings
        @new_settings[@setting_to_change] = to_numeric(@change_value)
        @new_settings = {"Settings" =>@new_settings}
        File.write(Settings.smith_settings_file, JSON.pretty_generate(@new_settings))
        #Printer.write_settings_file(@new_settings)
        Printer.send_command(CMD_REFRESH_SETTINGS)
        #erb :change_value
        redirect :edit
      end

      def to_numeric(anything)
        num = BigDecimal.new(anything.to_s)
        if num.frac == 0
          num.to_i
        else
          num.to_f
        end
      end
    end
  end
end
