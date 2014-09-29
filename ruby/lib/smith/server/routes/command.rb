module Smith
  module Server
    class Application < Sinatra::Base

      helpers do
        def validate_command(command)
          return unless command.nil? || command.strip.empty?
          halt 400, { error: 'Command parameter is requred' }.to_json
        end
      end

      post '/command' do
        content_type 'application/json'
        command = params[:command]
        validate_command(command)

        begin
          if command.downcase.strip == Printer::Commands::CMD_GET_STATUS.downcase
            { command: command, response: printer.get_status }.to_json
          else
            printer.send_command(command)
            { command: command }.to_json
          end
        rescue Smith::Printer::CommunicationError => e
          halt 500, { error: e.message }.to_json
        ensure
          printer.close_command_response_pipe
        end

      end

    end
  end
end
