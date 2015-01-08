module Smith
  module Server
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
            { command: command, response: printer.get_status }.to_json
          else
            printer.send_command(command)
            { command: command }.to_json
          end
        rescue Smith::Printer::CommunicationError => e
          halt 500, { error: e.message }.to_json
        end

      end

    end
  end
end
