# This is configuration shared by all tests

require 'bundler/setup'

if ENV['COVERAGE']
  require 'simplecov'
  SimpleCov.start
end

Dir[File.expand_path('../support/**/*.rb', __FILE__)].each { |f| require(f) }

RSpec.configure do |config|
  config.alias_example_to(:scenario)

  config.include(FileHelper, :tmp_dir)

  config.before(:each, :tmp_dir) do
    require 'tmpdir'
    @tmp_dir_path = File.expand_path("#{Dir.tmpdir}/#{Time.now.to_i}#{rand(1000)}/")
    FileUtils.mkdir_p(@tmp_dir_path)
    ENV['WPA_ROAM_DIR'] = @tmp_dir_path
    ENV['HOSTAPD_CONF_DIR'] = @tmp_dir_path
    ENV['DNSMASQ_CONF_DIR'] = @tmp_dir_path
    ENV['STORAGE_DIR'] = @tmp_dir_path
    ENV['LOG_DIR'] = @tmp_dir_path
  end

  config.after(:each, :tmp_dir) do
    FileUtils.rm_rf(@tmp_dir_path) if File.exists?(@tmp_dir_path)
  end

  config.order = 'random'
end
