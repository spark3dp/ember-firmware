module Smith
  module ConfigApp
    class App < Sinatra::Base

      get '/wireless' do
	@available_networks = Config::Network.available_wireless_networks
	erb :wireless
      end
    end
  end
end
