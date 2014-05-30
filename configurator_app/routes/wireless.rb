class ConfiguratorApp < Sinatra::Base

  get '/wireless' do
    @available_networks = Configurator.available_networks
    erb :wireless
  end
end