require 'config_helper'

module Smith::Config
  describe 'Command line firmware upgrade utility' do
    it 'starts firmware upgrade' do
      expect(Firmware).to receive(:upgrade).with('package_file')

      CLI.start(['upgrade', 'package_file'])
    end
  end
end
