#  File: cli.rb
#  Command line interface for interacting with configuration utility
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

require 'thor'

module Smith
  module Config
    class CLI < Thor

      desc 'load FILE', 'Load configuration from FILE'
      def load(file_path)
        Network.configure_from_file(file_path)
      end

      desc 'mode MODE', 'Configure the wireless adapter to operate in managed or ap MODE'
      def mode(mode)
        case mode
        when 'managed'
          WirelessInterface.enable_managed_mode
        when 'ap'
          Network.enable_ap_mode
        else
          puts 'MODE must be managed or ap'
        end
      rescue Smith::Config::System::Error =>e
        STDERR.puts(e.message)
        exit(1)
      end

      desc 'upgrade PACKAGE_PATH', 'Upgrade firmware with upgrade package located at PACKAGE_PATH'
      def upgrade(package_path)
        Firmware.upgrade(package_path)
      end
  
      desc 'version', 'Get firmware version'
      def version
        puts VERSION
      end

      desc 'init', 'Enable access point mode if wireless network configuration is not present'
      def init
        Network.init
      end

    end
  end
end
