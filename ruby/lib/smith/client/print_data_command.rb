# Class to handle print_data command
# Downloads file specified in command payload and sends commands necessary to
# apply settings and process print data

module Smith
  module Client
    class PrintDataCommand < Command

      def handle
        # Send acknowledgement to server with empty message
        acknowledge_command(:received)
        # Only start a download if the printer is in the home state
        @printer.validate_state { |state, substate| state == HOME_STATE }
      rescue Printer::InvalidState => e
        Client.log_error(LogMessages::PRINTER_NOT_READY_FOR_DATA, e.message)
        acknowledge_command(:failed, LogMessages::EXCEPTION_BRIEF, e)
      else
        # This runs if no exceptions were raised
        EM.next_tick do
          # Purge the print data directory
          # smith expects this directory to contain a single file when it receives the process print data command
          # There may be a file left in the directory as a result of an error
          @printer.purge_print_data_dir

          # Open a new file for writing in the print data directory with the same name as the last component in the file url
          @file = File.open(File.join(Settings.print_data_dir, @payload.file_url.split('/').last), 'wb')
          download_request = @http_client.get_file(@payload.file_url, @file)

          download_request.errback { acknowledge_command(:failed, LogMessages::PRINT_DATA_DOWNLOAD_ERROR, @payload.file_url) }
          download_request.callback { download_completed }
        end
      end

      private

      def download_completed
        Client.log_info(LogMessages::PRINT_DATA_DOWNLOAD_SUCCESS, @payload.file_url, @file.path)

        # Save print settings to temp file so smith can load them during processing
        File.write(Settings.print_settings_file, @payload.settings.to_json)
        
        # Send commands to load and process downloaded print data
        @printer.load_print_data
        @printer.process_print_data

        acknowledge_command(:completed)
      rescue StandardError => e
        Client.log_error(LogMessages::PRINT_DATA_LOAD_ERROR, e)
        acknowledge_command(:failed, LogMessages::EXCEPTION_BRIEF, e)
      end

    end
  end
end
