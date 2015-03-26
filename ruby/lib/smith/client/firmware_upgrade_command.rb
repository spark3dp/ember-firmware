# Class to handle firmware_upgrade command
# Downloads package file specified in command payload and applies upgrade

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
