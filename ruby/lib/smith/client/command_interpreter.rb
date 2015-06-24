# Class to interpret incoming web commands and delegate to the appropriate command object

require 'json'

require 'smith/client/print_data_command'
require 'smith/client/settings_command'
require 'smith/client/request_settings_command'
require 'smith/client/logs_command'
require 'smith/client/firmware_upgrade_command'

module Smith
  module Client
    class CommandInterpreter

      # Map between commands and the command class that handles it
      # Any commands without an explicit entry are handled by the PrintEngineCommand
      COMMAND_CLASS_MAP = {
        PRINT_DATA_COMMAND =>       :PrintDataCommand,
        SETTINGS_COMMAND =>         :SettingsCommand,
        REQUEST_SETTINGS_COMMAND => :RequestSettingsCommand,
        LOGS_COMMAND =>             :LogsCommand,
        FIRMWARE_UPGRADE_COMMAND => :FirmwareUpgradeCommand
      }

      def initialize(state, http_client)
        @state, @http_client = state, http_client
      end

      def interpret(payload)
        Client.log_info(LogMessages::RECEIVE_COMMAND, payload)
        command_class(payload[:command]).new(@state, @http_client, Context.new(payload)).handle
      end

      private

      def command_class(command)
        # Retrieve the class corresponding to the command
        # Return PrintEngineCommand if the command is not in the command class map
        Client.const_get(COMMAND_CLASS_MAP.fetch(command, :PrintEngineCommand))
      end

    end
  end
end
