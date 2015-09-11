#  File: wireless_helper.rb
#  Functionality for formatting wireless network configuration pages
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
    module WirelessHelper

      def format_mode(mode)
        if mode == 'Master'
          'Infrastructure'
        else
          mode
        end
      end
      
      def format_security(security)
        case security
        when 'wpa-personal'
          'WPA Personal (PSK)'
        when 'wpa-enterprise'
          'WPA Enterprise (EAP)'
        when 'none'
          'None'
        else
          security.upcase
        end
      end

    end
  end
end
