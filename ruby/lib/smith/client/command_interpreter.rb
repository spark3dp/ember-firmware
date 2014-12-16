# Class to interpret incoming web commands and delegate to the appropriate command object

require 'json'
require 'ostruct'

require 'smith/client/print_data_command'
require 'smith/client/logs_command'
require 'smith/client/firmware_upgrade_command'

module Smith
  module Client
    class CommandInterpreter

      # Map between commands and the command class that handles it
      # Any commands without an explicit entry are handled by the PrintEngineCommand
      COMMAND_CLASS_MAP = {
        PRINT_DATA_COMMAND =>       :PrintDataCommand,
        LOGS_COMMAND =>             :LogsCommand,
        FIRMWARE_UPGRADE_COMMAND => :FirmwareUpgradeCommand
      }

      def initialize(printer, state, http_client)
        @printer, @state, @http_client = printer, state, http_client
      end

      def interpret(raw_payload)
        Client.log_info(LogMessages::RECEIVE_COMMAND, raw_payload)
        payload = JSON.parse(raw_payload, symbolize_names: true)
        command_class(payload[:command]).new(@printer, @state, @http_client, OpenStruct.new(payload)).handle
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
