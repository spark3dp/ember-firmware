module Smith
  module ConfigApp
    class App < Sinatra::Base
      get '/' do
	erb :main
      end
    end
  end
end
