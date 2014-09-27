module FileHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      require 'rspec/em'
      include FileHelper

      steps = RSpec::EM.async_steps do

        def remove_tmp_dir_async(&callback)
          remove_tmp_dir
          callback.call
        end

        def make_tmp_dir_async(&callback)
          make_tmp_dir
          callback.call
        end

      end

      include steps
    end
  end

end
