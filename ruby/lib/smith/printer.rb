module Smith
  class Printer
    class CommunicationError < StandardError; end
    class InvalidState < StandardError; end

    module Commands
      GET_STATUS                        = 'GetStatus'
      START_PRINT_DATA_LOAD             = 'StartPrintDataLoad'
      PROCESS_PRINT_DATA                = 'ProcessPrintData'
      PRIMARY_REGISTRATION_SUCCEEDED    = 'PrimaryRegistrationSucceeded'
      DISPLAY_PRIMARY_REGISTRATION_CODE = 'DisplayPrimaryRegistrationCode'
    end

    def send_command(command)
      raise(Errno::ENOENT) unless File.pipe?(Smith.command_pipe)
      Timeout::timeout(0.1) { File.write(Smith.command_pipe, command + "\n") }
    rescue Timeout::Error, Errno::ENOENT => e
      raise(CommunicationError, "Unable to communicate with printer: #{e.message}")
    end

    def command_response_pipe
      @command_response_pipe ||= Timeout::timeout(0.1) { File.open(Smith.command_response_pipe, 'r') }
    rescue Timeout::Error, Errno::ENOENT => e
      raise(CommunicationError, "Unable to communicate with printer: #{e.message}")
    end

    def close_command_response_pipe
      @command_response_pipe.close if @command_response_pipe
    end

    def get_status
      send_command(Commands::GET_STATUS)
      JSON.parse(read_command_response_pipe)[PRINTER_STATUS_KEY]
    end

    def read_command_response_pipe
      Timeout::timeout(0.1) { command_response_pipe.gets }
    rescue Timeout::Error => e
      raise(CommunicationError, "Did not receive response from printer: #{e.message}")
    end

    def validate_state(&condition)
      state, uisubstate = get_status.values_at(STATE_PS_KEY, UISUBSTATE_PS_KEY)
      if !condition.call(state, uisubstate)
        raise(InvalidState, "Printer state (state: #{state.inspect}, substate: #{uisubstate.inspect}) invalid")
      end
    end

    def purge_download_dir
      Dir[File.join(Smith.print_file_upload_dir, '*.tar.gz')].each { |f| File.delete(f) }
    end

  end
end
