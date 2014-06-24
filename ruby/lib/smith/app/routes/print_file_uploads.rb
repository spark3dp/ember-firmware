module Smith
  module App
    class Application < Sinatra::Base

      helpers do
        def send_command
          raise(Errno::ENOENT) unless File.pipe?(Smith.command_pipe)
          Timeout::timeout(0.1) { File.write(Smith.command_pipe, "LoadFile\n") }
        rescue Timeout::Error, Errno::ENOENT
          flash.now[:error] = 'Unable to communicate with print engine'
          halt erb :new_print_file_upload
        end

        def copy_print_file(print_file)
          File.open(File.join(Application.upload_path, print_file[:filename]), 'wb') do |file|
            file.write(print_file[:tempfile].read)
          end
        end
      end

      get '/print_file_uploads/new' do
        erb :new_print_file_upload
      end

      post '/print_file_uploads' do
        if params[:print_file]
          copy_print_file(params[:print_file])
          send_command
          erb :successful_print_file_upload
        else
          flash.now[:error] = 'Please select a print file'
          erb :new_print_file_upload
        end
      end

    end
  end
end
