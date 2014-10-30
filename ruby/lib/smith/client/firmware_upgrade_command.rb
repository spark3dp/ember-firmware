# Class to handle firmware_upgrade command
# Downloads package file specified in command payload and applys upgrade

require 'tempfile'

module Smith
  module Client
    class FirmwareUpgradeCommand < Command

      def handle
        # Send acknowledgement to server with empty message
        acknowledge_command(:received)
        
        EM.next_tick do
          # Open a new file for writing in the print data directory with
          # the same name as the last component in the file url
          @file = Tempfile.new('firmware_upgrade_package')
          download_request = download_file(@payload.package_url, @file)

          download_request.errback { download_failed }
          download_request.callback { download_completed }
        end
      end

      private

      def download_completed
        Client.log_info("Firmware package download from #{@payload.package_url.inspect} complete, file downloaded to #{@file.path.inspect}")
        Config::Firmware.upgrade(@file.path)
        acknowledge_command(:completed)
      rescue StandardError => e
        Client.log_error("Firmware upgrade failed: \n#{e.backtrace.first}: #{e.message} (#{e.class})\n#{e.backtrace.drop(1).map{|s| "\t#{s}"}.join("\n")}")
        acknowledge_command(:failed, "#{e.message} (#{e.class})")
      end

      def download_failed
        Client.log_error(message = "Error downloading firmware package from #{@payload.package_url.inspect}")
        acknowledge_command(:failed, message)
      end

    end
  end
end
