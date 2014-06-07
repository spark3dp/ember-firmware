module Smith
  module ConfigApp
    class App < Sinatra::Base

      get '/wireless_networks' do
        @available_networks = Config::Network.available_wireless_networks
        erb :wireless_networks
      end

      post '/wireless_networks/connect' do
        @wireless_network = Config::WirelessNetwork.new(JSON.parse(params[:wireless_network], symbolize_names: true))
        
        # Sleep and configure the wireless adapter asynchronosly to allow a response to be
        # returned to the client before the adhoc network is brought down
        Thread.new do
          sleep settings.wireless_connection_delay
          Config::Network.configure(@wireless_network)
        end

        erb :connect_wireless_network
      end

    end
  end
end
