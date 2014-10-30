# Class to interpret incoming web commands and delegate to the appropriate command object

require 'json'
require 'ostruct'

module Smith
  module Client
    class CommandInterpreter

      # Map between commands and the command class that handles it
      # Any commands without an explict entry are handled by the PrintEngineCommand
      COMMAND_CLASS_MAP = {
        print_data:       :PrintDataCommand,
        logs:             :LogsCommand,
        firmware_upgrade: :FirmwareUpgradeCommand
      }

      def initialize(printer, state)
        @printer, @state = printer, state
      end

      def interpret(raw_payload)
        Client.log_info("Received command message from server containing #{raw_payload}")
        payload = JSON.parse(raw_payload, symbolize_names: true)
        Client.log_debug("Successfully parsed command notification payload: #{payload.inspect}")
        command_class(payload[:command]).new(@printer, @state, OpenStruct.new(payload)).handle
      end

      private

      def command_class(command)
        # Retrieve the class corresponding to the command
        # Return PrintEngineCommand if the command is not in the command class map
        Client.const_get(COMMAND_CLASS_MAP.fetch(command.to_sym, :PrintEngineCommand))
      end

    end
  end
end
