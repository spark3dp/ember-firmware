module Smith
  module Server
    class Application < Sinatra::Base

      get '/' do
        #erb :index
        redirect to '/print_file_uploads/new'
      end

      # Handle captive portal test for OSX
      get '/library/test/success.html' do
        redirect to '/wireless_networks'
      end

    end
  end
end
