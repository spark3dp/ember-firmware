module FirmwareUpgradeHelperAsync

  def self.included(including_class)
    including_class.class_exec do
      include FirmwareUpgradeHelper

      steps = RSpec::EM.async_steps do
        def setup_firmware_upgrade_async(&callback)
          setup_firmware_upgrade
          set_one_entry_versions_file
          callback.call
        end
      end

      include steps
    end
  end

end
