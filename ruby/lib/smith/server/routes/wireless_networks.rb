require 'timeout'

module Smith
  module Server
    class Application < Sinatra::Base

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

        begin
          Printer.validate_not_in_downloading_or_loading
          Printer.show_wireless_connecting
        rescue Printer::CommunicationError, Printer::InvalidState => e
          flash.now[:error] = e.message
          erb :connect_wireless_network
        else 
          # Sleep and configure the wireless adapter asynchronously to allow a response to be
          # returned to the client before switching off access point mode
          Thread.new do
            sleep Settings.wireless_connection_delay
            Config::Network.enter_managed_mode(@wireless_network)
          end
          flash.now[:info] = "Now attempting to connect to \"#{@wireless_network.ssid}\""
          erb :connect_wireless_network
        end

      end

    end
  end
end
