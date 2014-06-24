ENV['RACK_ENV'] = 'test'

require 'smith/config'
require 'smith/app/application'

require 'capybara'
require 'capybara/dsl'

Capybara.app = Smith::App::Application.new
Capybara.default_host = Capybara.app.settings.canonical_host

Dir[File.expand_path('../support/**/*.rb', __FILE__)].each { |f| require(f) }

RSpec.configure do |config|
  config.alias_example_to(:scenario)

  config.include(FileHelper, :tmp_dir)

  config.before(:each, :tmp_dir) do
    require 'tmpdir'
    @tmp_dir_path = File.expand_path("#{Dir.tmpdir}/#{Time.now.to_i}#{rand(1000)}/")
    FileUtils.mkdir_p(@tmp_dir_path)
    ENV['WPA_ROAM_PATH'] = @tmp_dir_path
    ENV['HOSTAPD_CONF_PATH'] = @tmp_dir_path
    ENV['DNSMASQ_CONF_PATH'] = @tmp_dir_path
    ENV['STORAGE_PATH'] = @tmp_dir_path
    ENV['UPLOAD_PATH'] = @tmp_dir_path
  end

  config.after(:each, :tmp_dir) do
    FileUtils.rm_rf(@tmp_dir_path) if File.exists?(@tmp_dir_path)
  end

  config.order = 'random'

  config.include Capybara::DSL
end
