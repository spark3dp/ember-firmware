#  File: application.rb
#  Top level Sinatra application definition
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
