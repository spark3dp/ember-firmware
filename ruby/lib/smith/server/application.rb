require 'sinatra/base'
require 'sinatra/partial'
require 'sinatra/contrib'
require 'rack-flash'

require 'smith/config'
require 'smith/printer'

module Smith
  module Server
    class Application < Sinatra::Base

      register Sinatra::Partial
      register Sinatra::RespondWith
      use Rack::Flash

      set :app_file, __FILE__
      set :partial_template_engine, :erb
      set :bind, '0.0.0.0'
      set :erb, escape_html: true

      enable :partial_underscores
      enable :sessions
      enable :logging

      configure :test do
        # wireless_connection_delay is how long to wait after processing web request to connect to wireless network
        # but before initiating the connection process
        set :canonical_host, 'http://webapp.com'
      end

      configure :development do
        set :canonical_host, 'http://localhost'
        register Sinatra::Reloader
        also_reload File.join(root, 'helpers/**/*.rb')
        also_reload File.join(Smith.root, 'lib/smith/config/**/*.rb')
        also_reload File.join(Smith.root, 'lib/smith/*.rb')
        also_reload File.join(Smith.root, 'lib/*.rb')
      end

      configure :production do
        set :canonical_host, "http://#{Config::WirelessInterface.ap_mode_ip_address}"
        set :port, 80
      end

      #before do
        #redirect "#{settings.canonical_host}:#{settings.port.to_s + request.path_info}", 302 if "http://#{request.host}" != settings.canonical_host
      #end
    end
  end
end

require_relative 'routes/init'
require_relative 'helpers/init'
