# Provides interface for interacting with smith via named pipes and other files

module Smith
  module Printer
    class CommunicationError < StandardError; end
    class InvalidState < StandardError; end

    module_function

    def serial_number
      %x(hexdump -e '8/1 "%c"' /sys/bus/i2c/devices/0-0050/eeprom -s 16 -n 12)
    end

    def show_downloading
      send_command(CMD_SHOW_PRINT_DATA_DOWNLOADING)
    end

    def show_download_failed
      send_command(CMD_SHOW_PRINT_DOWNLOAD_FAILED)
    end

    def show_loading
      send_command(CMD_START_PRINT_DATA_LOAD)
    end

    def process_print_data
      send_command(CMD_PROCESS_PRINT_DATA)
    end

    def apply_settings_file
      send_command(CMD_APPLY_SETTINGS)
    end

    def show_loaded
      send_command(CMD_SHOW_PRINT_DATA_LOADED)
    end

    def show_wireless_connecting
      send_command(CMD_SHOW_WIRELESS_CONNECTING)
    end

    def show_wireless_connected
      send_command(CMD_SHOW_WIRELESS_CONNECTED)
    end

    def show_wireless_connection_failed
      send_command(CMD_SHOW_WIRELESS_CONNECTION_FAILED)
    end

    def validate_not_in_downloading_or_loading
      validate_state do |state, substate|
        state == HOME_STATE &&
          substate != LOADING_PRINT_DATA_SUBSTATE &&
          substate != DOWNLOADING_PRINT_DATA_SUBSTATE
      end
    end

    def send_command(command)
      raise(Errno::ENOENT) unless File.pipe?(Settings.command_pipe)
      Timeout::timeout(Settings.printer_communication_timeout) do
        File.write(Settings.command_pipe, command + "\n")
      end
    rescue Timeout::Error, Errno::ENOENT => e
      raise(CommunicationError, "Unable to send command '#{command}' to printer: #{e.message}")
    end

    def get_status
      Timeout::timeout(Settings.printer_communication_timeout) do
        read_printer_status_file
      end
    rescue Timeout::Error, Errno::ENOENT => e
      raise(CommunicationError, "Unable to read valid JSON from printer status file: #{e.message}")
    end

    def read_printer_status_file
      JSON.parse(File.read(Settings.printer_status_file).sub("\n", ''))
    rescue JSON::ParserError
      read_printer_status_file
    end

    def validate_state(&condition)
      state, uisubstate = get_status.values_at(STATE_PS_KEY, UISUBSTATE_PS_KEY)
      if !condition.call(state, uisubstate)
        raise(InvalidState, "Printer state (state: #{state.inspect}, ui_sub_state: #{uisubstate.inspect}) invalid")
      end
    end

    def purge_print_data_dir
      Dir[File.join(Settings.print_data_dir, '*.tar.gz')].each { |f| File.delete(f) }
    end

    def current_print_file
      JSON.parse(File.read(Settings.smith_settings_file)).fetch(SETTINGS_ROOT_KEY).fetch(PRINT_FILE_SETTING)
    end

    def write_settings_file(settings)
      File.write(Settings.settings_file, settings.to_json)
    end

    def write_registration_info_file(info)
      File.write(Settings.registration_info_file, info.to_json)
    end

  end
end
