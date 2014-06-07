module Smith
  module ConfigApp
    class App < Sinatra::Base
      get '/' do
        erb :index
      end
    end
  end
end
