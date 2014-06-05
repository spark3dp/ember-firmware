require 'sinatra/base'
require 'sinatra/partial'

require 'smith'

Tilt.register Tilt::ERBTemplate, 'html.erb'

module Smith
  module ConfigApp
    class App < Sinatra::Base

      register Sinatra::Partial

      set :app_file, __FILE__
      set :partial_template_engine, :erb
      set :bind, '0.0.0.0'

      enable :partial_underscores

      configure :development do
	set :port, 4567
      end

      configure :production do
	set :port, 80
      end

    end
  end
end

require_relative 'routes/init'
require_relative 'helpers/init'
