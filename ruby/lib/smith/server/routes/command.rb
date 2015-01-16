module Smith
  module Server

    # Commands supported by server that are handled directly instead of through smith
    CMD_GET_STATUS = 'GETSTATUS'
    CMD_GET_BOARD_NUM = 'GETBOARDNUM'
    CMD_GET_FW_VERSION = 'GETFWVERSION'

    class Application < Sinatra::Base

      helpers do
        def validate_command(command)
          return unless command.nil? || command.strip.empty?
          halt 400, { error: 'Command parameter is required' }.to_json
        end
      end

      post '/command' do
        content_type 'application/json'
        command = params[:command]
        validate_command(command)

        begin
          case command.upcase.strip
          when CMD_GET_STATUS
            { command: command, response: Printer.get_status }.to_json
          when CMD_GET_BOARD_NUM
            { command: command, response: Printer.serial_number }.to_json
          when CMD_GET_FW_VERSION
            { command: command, response: VERSION }.to_json
          else
            Printer.send_command(command)
            { command: command }.to_json
          end
        rescue Smith::Printer::CommunicationError => e
          halt 500, { error: e.message }.to_json
        end

      end

    end
  end
end
