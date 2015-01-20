require 'config_helper'
require 'fileutils'

module Smith::Config
  describe 'Network', :tmp_dir do

    scenario 'initialize wireless network when wpa-roam config file exists' do
      FileUtils.touch(wpa_roam_file)

      expect(Network).not_to receive(:enable_ap_mode)

      CLI.start(['init'])
    end

    scenario 'initialize wireless network when wpa-roam config file does not exist' do
      expect(Network).to receive(:enable_ap_mode)

      CLI.start(['init'])
    end

  end
end
