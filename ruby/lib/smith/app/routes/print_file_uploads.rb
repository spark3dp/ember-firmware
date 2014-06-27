module Smith
  module App
    class Application < Sinatra::Base

      helpers do
        def send_command(command)
          raise(Errno::ENOENT) unless File.pipe?(Smith.command_pipe)
          Timeout::timeout(0.1) { File.write(Smith.command_pipe, command + "\n") }
        rescue Timeout::Error, Errno::ENOENT
          flash.now[:error] = 'Unable to communicate with print engine'
          halt erb :new_print_file_upload
        end

        def copy_print_file
          File.open(File.join(Application.upload_path, @print_file[:filename]), 'wb') do |file|
            file.write(@print_file[:tempfile].read)
          end
        end

        def validate_print_file
          return if @print_file
          flash.now[:error] = 'Please select a print file'
          halt erb :new_print_file_upload
        end

        def get_printer_status
          send_command('GETSTATUS')
          JSON.parse(Timeout::timeout(0.1) { File.open(Smith.command_response_pipe) { |f| f.gets } })[PRINTER_STATUS_KEY]
        rescue Timeout::Error, Errno::ENOENT
          flash.now[:error] = 'Unable to communicate with print engine'
          halt erb :new_print_file_upload
        end

        def validate_printer_status(printer_status)
          state = printer_status[STATE_PS_KEY]
          return if ['Home', 'Homing', 'Idle'].include?(state)
          flash.now[:error] = "Printer cannnot accept file while in #{state} state"
          halt erb :new_print_file_upload
        end
      end

      get '/print_file_uploads/new' do
        erb :new_print_file_upload
      end

      post '/print_file_uploads' do
        @print_file = params[:print_file]

        validate_print_file
        validate_printer_status(get_printer_status)
        copy_print_file
        send_command('SETPRINTDATA')

        erb :successful_print_file_upload
      end

    end
  end
end
