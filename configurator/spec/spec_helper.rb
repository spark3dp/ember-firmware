require 'configurator'

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
end
