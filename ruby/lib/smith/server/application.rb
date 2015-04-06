require 'sinatra/base'
require 'sinatra/partial'
require 'sinatra/contrib'
require 'rack-flash'

require 'smith/config'
require 'smith/printer'

module Smith
  module Server
    class Application < Sinatra::Base

      def self.run!
        super do |server|
          # The default Thin timeout of 30s causes dropped connection when
          # handling firmware upgrade request; the upgrade process takes
          # longer than 30s
          server.timeout = 120
        end
      end

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

      configure :development do
        register Sinatra::Reloader
        also_reload File.join(root, 'helpers/**/*.rb')
        also_reload File.join(Smith.root, 'lib/smith/config/**/*.rb')
        also_reload File.join(Smith.root, 'lib/smith/*.rb')
        also_reload File.join(Smith.root, 'lib/*.rb')
      end

      configure :production do
        set :port, 80
      end

    end
  end
end

require_relative 'routes/init'
require_relative 'helpers/init'
