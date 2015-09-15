#  File: print_file_uploads.rb
#  Sinatra routes for loading print files
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

module Smith
  module Server
    class Application < Sinatra::Base

      VALID_FILE_REGEXES = [/\A.+?\.tar\.gz\z/i, /\A.+?\.zip\z/i]

      helpers do
        def copy_print_file
          FileUtils.copy(@print_file[:tempfile].path,
            File.join(Settings.print_data_dir, @print_file[:filename]))
        end

        def validate_print_file
          unless is_file_upload?(@print_file)
            flash.now[:error] = 'Please select a print file'
            respond_with :new_print_file_upload do |f|
              f.json { error 400, flash_json }
            end
          end

          if !@print_file[:filename].match(Regexp.union(VALID_FILE_REGEXES))
            flash.now[:error] = 'Please select a .tar.gz or .zip file'
            respond_with :new_print_file_upload do |f|
              f.json { error 400, flash_json }
            end
          end
        end

        def process_print_file_upload
          Printer.validate_not_in_downloading_or_loading
          Printer.purge_print_data_dir
          Printer.show_loading
          copy_print_file
          Printer.process_print_data
        rescue Smith::Printer::CommunicationError, Smith::Printer::InvalidState => e
          flash.now[:error] = e.message
          respond_with :new_print_file_upload do |f|
            f.json { error 500, flash_json }
          end
        ensure
          @print_file[:tempfile].close!
        end

      end

      get '/print_file_uploads/new' do
        erb :new_print_file_upload
      end

      post '/print_file_uploads', provides: [:html, :json] do
        @print_file = params[:print_file]
        validate_print_file
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
