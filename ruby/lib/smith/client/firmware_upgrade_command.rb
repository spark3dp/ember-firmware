#  File: firmware_upgrade_command.rb
#  Handles firmware upgrade command by downloading package specified in
#  command payload and applying the upgrade
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

require 'tempfile'

module Smith
  module Client

    FIRMWARE_UPGRADE_COMMAND = 'firmware_upgrade'

    class FirmwareUpgradeCommand < Command

      def handle
        # Send acknowledgement to server with empty message
        acknowledge_command(Command::RECEIVED_ACK)
        
        EM.next_tick do
          # Open a temp file to store the package contents
          @file = Tempfile.new('firmware_upgrade_package')
          download_request = @http_client.get_file(@payload.package_url, @file, redirects: 1)

          download_request.errback { acknowledge_command(Command::FAILED_ACK, LogMessages::FIRMWARE_DOWNLOAD_ERROR, @payload.package_url) }
          download_request.callback { download_completed }
        end
      end

      private

      def download_completed
        Client.log_info(LogMessages::FIRMWARE_DOWNLOAD_SUCCESS, @payload.package_url, @file.path)
        Config::Firmware.upgrade(@file.path)
        Client.log_info(LogMessages::FIRMWARE_UPGRADE_SUCCESS)
        acknowledge_command(Command::COMPLETED_ACK)
      rescue StandardError => e
        Client.log_error(LogMessages::FIRMWARE_UPGRADE_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, LogMessages::EXCEPTION_BRIEF, e)
      ensure
        @file.close!
      end

    end
  end
end
