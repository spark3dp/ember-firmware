# Class to handle print_data command
# Downloads file specified in command payload and sends commands necessary to
# apply settings and process print data

module Smith
  module Client

    PRINT_DATA_COMMAND = 'print_data'

    class PrintDataCommand < Command

      def handle
        # Send acknowledgement to server with empty message
        acknowledge_command(Command::RECEIVED_ACK)

        # Use the last component in the file URL as the file name
        @file_name = @payload.file_url.split('/').last

        # Read the currently loaded print file from the smith settings file
        current_file = Printer.current_print_file

        if current_file == @file_name
          # The file at the specified URL is currently loaded by smith
          # Only apply the specified settings
          Client.log_info(LogMessages::PRINT_DATA_LOAD_FILE_CURRENTLY_LOADED, @file_name, current_file)
          apply_settings
        else
          # The file at the specified URL is not currently loaded
          # Download the file and command smith to process it
          Client.log_info(LogMessages::PRINT_DATA_LOAD_FILE_NOT_CURRENTLY_LOADED, @file_name, current_file)
          start_download
        end
      end

      private

      def apply_settings
        Printer.validate_state { |state| state == HOME_STATE }
        Printer.show_loading
        if @payload.job_id
          @payload.settings[SETTINGS_ROOT_KEY.to_sym][JOB_ID_SETTING.to_sym] = @payload.job_id
        end
        Printer.write_settings_file(@payload.settings)
        Printer.apply_settings_file
        Printer.show_loaded
        acknowledge_command(Command::COMPLETED_ACK)
      rescue StandardError => e
        Client.log_error(LogMessages::PRINT_DATA_LOAD_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, LogMessages::EXCEPTION_BRIEF, e)
      end

      def start_download
        Printer.validate_not_in_downloading_or_loading
      rescue Printer::InvalidState, Printer::CommunicationError => e
        Client.log_error(LogMessages::PRINTER_NOT_READY_FOR_DATA, e.message)
        acknowledge_command(Command::FAILED_ACK, LogMessages::EXCEPTION_BRIEF, e)
      else
        # This runs if no exceptions were raised
        EM.next_tick do
          Printer.show_downloading
          # Purge the print data directory
          # smith expects this directory to contain a single file when it receives the process print data command
          # There may be a file left in the directory as a result of an error
          Printer.purge_print_data_dir

          # Open a new file for writing in the print data directory
          @file = File.open(File.join(Settings.print_data_dir, @file_name), 'wb')
          download_request = @http_client.get_file(@payload.file_url, @file)

          download_request.errback { download_failed }
          download_request.callback { download_completed }
        end
      end

      def download_completed
        Client.log_info(LogMessages::PRINT_DATA_DOWNLOAD_SUCCESS, @payload.file_url, @file.path)

        # Save print settings to temp file so smith can load them during processing
        if @payload.job_id
          @payload.settings[SETTINGS_ROOT_KEY.to_sym][JOB_ID_SETTING.to_sym] = @payload.job_id
        end
        Printer.write_settings_file(@payload.settings)

        # Send commands to load and process downloaded print data
        Printer.validate_state { |state| state == HOME_STATE }
        Printer.show_loading
        Printer.process_print_data

        acknowledge_command(Command::COMPLETED_ACK)
      rescue StandardError => e
        Client.log_error(LogMessages::PRINT_DATA_LOAD_ERROR, e)
        acknowledge_command(Command::FAILED_ACK, LogMessages::EXCEPTION_BRIEF, e)
      end

      def download_failed
        acknowledge_command(Command::FAILED_ACK, LogMessages::PRINT_DATA_DOWNLOAD_ERROR, @payload.file_url)
        Printer.show_download_failed
      end

      # Send an command acknowledgement that includes the job_id.
      # Post request state is the stage of the command acknowledgment
      # If only the state is specified, the message is nil
      # If a string is specified as the third argument, it is formatted as a log message using any additional arguments
      # If something other than a string is specified, it is used directly
      def acknowledge_command(state, *args)
        m = message(*args)
        request = @http_client.post(acknowledge_endpoint(@payload), command_payload(@payload.command, state, m, Printer.get_status, @payload.job_id))
        request.callback { Client.log_debug(LogMessages::ACKNOWLEDGE_COMMAND, @payload.command, @payload.task_id, state, m) }
      end
    end
  end
end
