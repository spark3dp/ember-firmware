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
        printer.show_wireless_connecting
      
        # Sleep and configure the wireless adapter asynchronously to allow a response to be
        # returned to the client before switching off access point mode
        config_thread = Thread.new do
          sleep Settings.wireless_connection_delay
          begin
            Config::Network.configure(@wireless_network)
            Timeout::timeout(Settings.wireless_connection_timeout) do
              loop do
                break if Config::WirelessInterface.connected?
                sleep Settings.wireless_connection_poll_interval
              end
            end
          rescue Timeout::Error
            printer.show_wireless_connection_failed
          rescue StandardError => e
            $stderr.puts(e.inspect + "\n" + e.backtrace.map{ |l| l.prepend('      ') }.join("\n"))
          else
            printer.show_wireless_connected
          end
        end
        config_thread[:name] = :config

        erb :connect_wireless_network
      end

    end
  end
end
