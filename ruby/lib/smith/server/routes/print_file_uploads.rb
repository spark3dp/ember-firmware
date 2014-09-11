module Smith
  module Server
    class Application < Sinatra::Base

      helpers do
        def purge_upload_dir
          Dir[File.join(Application.upload_dir, '*.tar.gz')].each { |f| File.delete(f) }
        end

        def copy_print_file
          FileUtils.copy(@print_file[:tempfile].path, File.join(Application.upload_dir, @print_file[:filename]))
        end

        def validate_print_file
          if !(@print_file && @print_file.is_a?(Hash) && @print_file[:tempfile] && @print_file[:tempfile].respond_to?(:path))
            flash.now[:error] = 'Please select a print file'
            respond_with :new_print_file_upload do |f|
              f.json { error 400, flash_json }
            end
          end

          if @print_file[:filename] !~ /\A.+?\.tar\.gz\z/i
            flash.now[:error] = 'Please select a .tar.gz file'
            respond_with :new_print_file_upload do |f|
              f.json { error 400, flash_json }
            end
          end
        end

        def validate_printer_status(printer_status)
          state = printer_status[STATE_PS_KEY]
          substate = printer_status[UISUBSTATE_PS_KEY]
          return if (state == 'Home' and substate != 'DownloadFailed')
          flash.now[:error] = "Printer cannot accept file while in #{state} state and #{substate} substate"
          respond_with :new_print_file_upload do |f|
            f.json { error 500, flash_json }
          end
        end

        def process_print_file_upload
          validate_print_file
          purge_upload_dir
          validate_printer_status(printer.get_status)
          printer.send_command('STARTPRINTDATALOAD')
          validate_printer_status(printer.get_status)
          copy_print_file
          printer.send_command('PROCESSPRINTDATA')
        rescue Smith::Printer::CommunicationError => e
          flash.now[:error] = e.message
          respond_with :new_print_file_upload do |f|
            f.json { error 500, flash_json }
          end
        ensure
          printer.close_command_response_pipe
        end

      end

      get '/print_file_uploads/new' do
        erb :new_print_file_upload
      end

      post '/print_file_uploads', provides: [:html, :json] do
        @print_file = params[:print_file]
        process_print_file_upload 
        flash[:success] = 'Print file loaded successfully'
        respond do |f|
          f.html { redirect to '/print_file_uploads/new' }
          f.json { flash_json }
        end
      end

    end
  end
end
