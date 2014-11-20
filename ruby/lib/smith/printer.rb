# Provides interface for interacting with smith via named pipes and other files

module Smith
  class Printer
    class CommunicationError < StandardError; end
    class InvalidState < StandardError; end

    def show_loading
      validate_state { |state, substate| state == HOME_STATE && substate != LOADING_PRINT_DATA_SUBSTATE }
      send_command(CMD_START_PRINT_DATA_LOAD)
    end
    
    def process_print_data
      validate_state { |state, substate| state == HOME_STATE && substate == LOADING_PRINT_DATA_SUBSTATE }
      send_command(CMD_PROCESS_PRINT_DATA)
    end

    def apply_print_settings_file
      send_command(CMD_APPLY_PRINT_SETTINGS)
    end

    def show_loaded
      send_command(CMD_SHOW_PRINT_DATA_LOADED)
    end

    def send_command(command)
      raise(Errno::ENOENT) unless File.pipe?(Settings.command_pipe)
      Timeout::timeout(0.1) { File.write(Settings.command_pipe, command + "\n") }
    rescue Timeout::Error, Errno::ENOENT => e
      raise(CommunicationError, "Unable to communicate with printer: #{e.message}")
    end

    def command_response_pipe
      @command_response_pipe ||= Timeout::timeout(0.1) { File.open(Settings.command_response_pipe, 'r') }
    rescue Timeout::Error, Errno::ENOENT => e
      raise(CommunicationError, "Unable to communicate with printer: #{e.message}")
    end

    def close_command_response_pipe
      @command_response_pipe.close if @command_response_pipe
    end

    def get_status
      send_command(CMD_GET_STATUS)
      JSON.parse(read_command_response_pipe)
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

    def purge_print_data_dir
      Dir[File.join(Settings.print_data_dir, '*.tar.gz')].each { |f| File.delete(f) }
    end

    def current_print_file
      JSON.parse(File.read(Settings.smith_settings_file)).fetch(SETTINGS_ROOT_KEY).fetch(PRINT_FILE_SETTING)
    end

    def write_settings_file(settings)
      File.write(Settings.print_settings_file, settings.to_json)
    end

    def write_registration_info_file(info)
      File.write(Settings.registration_info_file, info.to_json)
    end

  end
end
