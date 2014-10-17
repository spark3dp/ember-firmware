require File.expand_path('../boot', __FILE__)

# Pick the frameworks you want:
require "active_model/railtie"
# require "active_record/railtie"
require "action_controller/railtie"
#require "action_mailer/railtie"
#require "action_view/railtie"
# require "sprockets/railtie"
# require "rails/test_unit/railtie"

# Require the gems listed in Gemfile, including any gems
# you've limited to :test, :development, or :production.
# Explictly require dependencies rather than having bundler pull in everything in the gemspec
#Bundler.require(*Rails.groups)

require 'faye-rails'

# Define authentication token, printer id and registration code as constants as this server is only used for testing
AUTH_TOKEN = 'authtoken'
PRINTER_ID = 539
REGISTRATION_CODE = '4321'

module DummyServer
  class Application < Rails::Application
    # Settings in config/environments/* take precedence over those specified here.
    # Application configuration should go into files in config/initializers
    # -- all .rb files in that directory are automatically loaded.

    # Set Time.zone default to the specified zone and make Active Record auto-convert to this zone.
    # Run "rake -D time" for a list of tasks for finding time zone names. Default is UTC.
    # config.time_zone = 'Central Time (US & Canada)'

    # The default locale is :en and all translations from config/locales/*.rb,yml are auto loaded.
    # config.i18n.load_path += Dir[Rails.root.join('my', 'locales', '*.{rb,yml}').to_s]
    # config.i18n.default_locale = :de

    config.middleware.delete Rack::Lock
    config.middleware.use FayeRails::Middleware, mount: '/faye', timeout: 25 do |faye|

      # Add server-side authentication extenstion to ensure client provides auth token with faye subscriptions
      faye.add_extension(ServerAuth.new)

      # Set up logging
      faye.on :handshake do |client_id|
        Rails.logger.info "[Faye Event] Client #{client_id} connected"
      end
      faye.on :subscribe do |client_id, channel|
        Rails.logger.info "[Faye Event] Client #{client_id} subscribed to #{channel}."
      end
      faye.on :unsubscribe do |client_id, channel|
        Rails.logger.info "[Faye Event] Client #{client_id} unsubscribed from #{channel}."
      end
      faye.on :publish do |client_id, channel, data|
        Rails.logger.info "[Faye Event] Client #{client_id} published #{data.inspect} to #{channel}."
      end
      faye.on :disconnect do |client_id|
        Rails.logger.info "[Faye Event] Client #{client_id} disconnected"
      end
    end
  end
end
