require 'bundler/setup'
Bundler.setup

require 'support/tmp_dir_helper'
require 'support/motor_controller_helper'

RSpec.configure do |config|
  config.alias_example_to(:scenario)

  config.order = 'random'

  config.include(Tests::TmpDirHelper)
  config.include(Tests::MotorControllerHelper)

  config.before(:each) do
    make_tmp_dir
  end

  config.after(:each) do
    remove_tmp_dir
  end
end
