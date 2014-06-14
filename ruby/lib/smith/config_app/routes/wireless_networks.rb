module Smith
  module ConfigApp
    class App < Sinatra::Base

      get '/wireless_networks' do
        @wireless_networks = Config::Network.available_wireless_networks
        @last_wireless_network = Config::WirelessNetwork.last_configured
        erb :wireless_networks
      end

      get '/wireless_networks/authenticate' do
        @wireless_network = Config::WirelessNetwork.new(params[:wireless_network])
        erb :authenticate_wireless_network
      end

      post '/wireless_networks/connect' do
        @wireless_network = Config::WirelessNetwork.new(params[:wireless_network])
        
        # Sleep and configure the wireless adapter asynchronosly to allow a response to be
        # returned to the client before switching off access point mode
        Thread.new do |thread|
          sleep settings.wireless_connection_delay
          begin
            Config::Network.configure(@wireless_network)
          rescue StandardError => e
            $stderr.puts(e.inspect + "\n" + e.backtrace.map{ |l| l.prepend('      ') }.join("\n"))
          end
        end

        erb :connect_wireless_network
      end

    end
  end
end
