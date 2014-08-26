module Smith
  module App
    class Application < Sinatra::Base

      helpers do
        def json_send_command(command)
          raise(Errno::ENOENT) unless File.pipe?(Smith.command_pipe)
          Timeout::timeout(0.1) { File.write(Smith.command_pipe, command + "\n") }
        rescue Timeout::Error, Errno::ENOENT
          body({ error: 'Unable to communicate with print engine' }.to_json)
          halt 500
        end

        def json_get_printer_status
          JSON.parse(Timeout::timeout(0.1) { @command_response_pipe.gets })[PRINTER_STATUS_KEY]
        rescue Timeout::Error => e
          body({ error: "Did not receive printer status: #{e.message}" }.to_json)
          halt 500
        end

        def json_open_command_response_pipe
          @command_response_pipe = Timeout::timeout(0.1) { File.open(Smith.command_response_pipe, 'r') }
        rescue Timeout::Error, Errno::ENOENT => e
          body({ error: "Unable to communicate with printer: #{e.message}" }.to_json)
          halt 500
        end

        def json_validate_command(command)
          return unless command.nil? || command.strip.empty?
          body({ error: 'Command parameter is requred' }.to_json)
          halt 400
        end
      end

      after '/command' do
        @command_response_pipe.close if @command_response_pipe
      end

      before '/command' do
        content_type 'application/json'
      end

      post '/command' do
        command = params[:command]
        json_validate_command(command)

        json_open_command_response_pipe if command.downcase == 'getstatus'
        json_send_command(command)

        if command.downcase == 'getstatus'
          { command: command, response: json_get_printer_status }.to_json
        else
          { command: command }.to_json
        end

      end

    end
  end
end
