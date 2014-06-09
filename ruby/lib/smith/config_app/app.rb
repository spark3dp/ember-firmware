require 'sinatra/base'
require 'sinatra/partial'
require 'sinatra/contrib'
require 'json'

require 'smith'

Tilt.register Tilt::ERBTemplate, 'html.erb'

module Smith
  module ConfigApp
    class App < Sinatra::Base

      register Sinatra::Partial

      set :app_file, __FILE__
      set :partial_template_engine, :erb
      set :bind, '0.0.0.0'
      set :erb, escape_html: true

      enable :partial_underscores

      configure :test do
        # wireless_connection_delay is how long to wait after processing web request to connect to wireless network
        # but before initiating the connection process
        set :wireless_connection_delay, 0
      end

      configure :development do
        set :wireless_connection_delay, 5 
        set :port, 4567
        register Sinatra::Reloader
        also_reload File.join(root, 'helpers/**/*.rb')
        also_reload File.join(Smith.root, 'lib/smith/config/**/*.rb')
      end

      configure :production do
        set :wireless_connection_delay, 5
        set :port, 80
      end

    end
  end
end

require_relative 'routes/init'
require_relative 'helpers/init'
