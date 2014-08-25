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
          File.open(File.join(Application.upload_dir, @print_file[:filename]), 'wb') do |file|
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
          JSON.parse(Timeout::timeout(0.1) { @command_response_pipe.gets })[PRINTER_STATUS_KEY]
        rescue Timeout::Error => e
          flash.now[:error] = "Did not receive printer status: #{e.message}"
          halt erb :new_print_file_upload
        end

        def validate_printer_status(printer_status)
          state = printer_status[STATE_PS_KEY]
          substate = printer_status[UISUBSTATE_PS_KEY]
          return if (state == 'Home' and substate != 'DownloadFailed')
          flash.now[:error] = "Printer cannot accept file while in #{state} state and #{substate} substate"
          halt erb :new_print_file_upload
        end

        def open_command_response_pipe
          @command_response_pipe = Timeout::timeout(0.1) { File.open(Smith.command_response_pipe, 'r') }
        rescue Timeout::Error, Errno::ENOENT => e
          flash.now[:error] = "Unable to communicate with printer: #{e.message}"
          halt erb :new_print_file_upload
        end
      end

      after '/print_file_uploads' do
        @command_response_pipe.close if @command_response_pipe
      end

      get '/print_file_uploads/new' do
        erb :new_print_file_upload
      end

      post '/print_file_uploads' do
        @print_file = params[:print_file]
        
        validate_print_file
        open_command_response_pipe
        validate_printer_status(get_printer_status)
        send_command('STARTPRINTDATALOAD')
        validate_printer_status(get_printer_status)
        copy_print_file
        send_command('PROCESSPRINTDATA')

        flash[:success] = 'Print file loaded successfully'
        redirect to '/print_file_uploads/new'
      end

    end
  end
end
