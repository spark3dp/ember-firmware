ENV['RACK_ENV'] = 'test'

require 'smith'
require 'smith/config_app/app'

require 'capybara'
require 'capybara/dsl'

Capybara.app = Smith::ConfigApp::App.new

Dir[File.expand_path('../support/**/*.rb', __FILE__)].each { |f| require(f) }

RSpec.configure do |config|
  config.treat_symbols_as_metadata_keys_with_true_values = true

  config.alias_example_to(:scenario)

  config.before(:each, :tmp_dir) do
    require 'tmpdir'
    @tmp_path = File.expand_path("#{Dir.tmpdir}/#{Time.now.to_i}#{rand(1000)}/")
    FileUtils.mkdir_p(@tmp_path)
  end

  config.after(:each, :tmp_dir) do
    FileUtils.rm_rf(@tmp_path) if File.exists?(@tmp_path)
  end

  config.order = 'random'

  config.include Capybara::DSL
end
