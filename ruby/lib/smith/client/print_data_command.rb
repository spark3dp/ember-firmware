# Class to handle print_data command
# Downloads file specified in command payload and sends commands necessary to
# apply settings and process print data

module Smith
  module Client
    class PrintDataCommand < Command

      def handle
        # Send acknowledgement to server with empty message
        acknowledge_command
        # Only start a download if the printer is in the home state
        @printer.validate_state { |state, substate| state == HOME_STATE }
      rescue Printer::InvalidState => e
        Client.log_error("#{e.message}, not downloading print data, aborting print_data command handling")
      else
        # This runs if no exceptions were raised
        EM.next_tick do

          # Purge the print data dir
          # smith expects this directory to contain a single file when it receives the process print data command
          # There may be a file left over as a result of an error
          @printer.purge_print_data_dir

          # Open a new file for writing in the print data directory with
          # the same name as the last component in the file url
          @file = File.open(File.join(Settings.print_data_dir, @payload.file_url.split('/').last), 'wb')
          download_request = EM::HttpRequest.new(@payload.file_url, connect_timeout: 25).get

          download_request.errback { download_failed }
          download_request.callback { download_completed }
          download_request.stream { |chunk| chunk_available(chunk) }

        end
      end

      private

      def download_completed
        Client.log_info("Print data download of #{@payload.file_url} complete, file downloaded to #{@file.path}")
        @file.close

        # Save print settings to temp file so smith can load them during processing
        File.write(Settings.print_settings_file, { SETTINGS_ROOT_KEY => @payload.settings }.to_json)
        
        # Send commands to load and process downloaded print data
        @printer.load_print_data
        @printer.process_print_data
      rescue Printer::InvalidState => e
        Client.log_error("#{e.message}, aborting print_data command handling")
      end

      def download_failed
        Client.log_error("Error downloading print data from #{@payload.file_url}")
        @file.close
      end

      def chunk_available(chunk)
        @file.write(chunk)
      end

    end
  end
end
