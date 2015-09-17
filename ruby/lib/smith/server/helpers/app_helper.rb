#  File: app_helper.rb
#  Sinatra helpers used throughout application
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
    module AppHelper

      def hidden_fields(object)
        fields = []
        object.marshal_dump.each_pair do |k, v|
          fields << %Q(<input type="hidden" name="wireless_network[#{k}]" value="#{v}" />)
        end 
        fields.join("\n")
      end

      # Clear the flash and return a json string of the contents
      def flash_json
        msg = Hash[flash.keys.map { |k| [k, flash[k]] }]
        # Clear the flash values
        flash.flag!
        flash.sweep!
        # Clear the flash cache
        flash.now.clear
        msg.to_json
      end

      def respond(&responder)
        respond_with('', &responder)
      end

      def is_file_upload?(file)
        file && file.is_a?(Hash) && file[:tempfile] && file[:tempfile].respond_to?(:path)
      end

    end
  end
end
