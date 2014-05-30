require 'sinatra/base'
require 'sinatra/partial'

require 'configurator'

Tilt.register Tilt::ERBTemplate, 'html.erb'

class ConfiguratorApp < Sinatra::Base

  register Sinatra::Partial

  set :app_file, __FILE__
  set :partial_template_engine, :erb

  enable :partial_underscores

end

require_relative 'routes/init'
