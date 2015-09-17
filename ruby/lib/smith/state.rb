#  File: state.rb
#  File backed implementation providing application state persistence functionality
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

require 'ostruct'
require 'json'

module Smith
  class State < OpenStruct

    class << self

      def load
        new(JSON.parse(File.read(Settings.state_file), symbolize_name: true))
      rescue Errno::ENOENT
        new
      end

    end

    def save
      File.write(Settings.state_file, marshal_dump.to_json)
    end

    def update(hash)
      hash.each do |key, value|
        send("#{key}=", value)
      end
      save
    end

    def get_binding
      binding
    end

  end
end
