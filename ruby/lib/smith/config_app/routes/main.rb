module Smith
  module ConfigApp
    class App < Sinatra::Base
      get '/' do
        erb :index
      end
      get '/library/test/success.html' do
        redirect to('http://192.168.1.1/')
      end
    end
  end
end
