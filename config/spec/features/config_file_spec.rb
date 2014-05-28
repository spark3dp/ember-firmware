require 'spec_helper'
require 'open3'

describe 'load_wifi_config', :tmp_dir do

  def load_wifi_config(config_file_name)
    executable = File.expand_path('../../../bin/load_wifi_config', __FILE__)
    config_file = File.join(File.expand_path('../../resources', __FILE__), config_file_name)
    env = { 'WPA_ROAM_PATH' => @tmp_path, 'WIFI_INTERFACE' => 'wlan0' }
    Open3.capture3(env, executable, config_file)
  end

  context 'when loading config file for unsecured network' do

    it 'configures wireless in managed mode' do
      expect(Network).to receive(:enable_managed_mode).with('wlan0')
      
      stdout_str, stderr_str, status = load_wifi_config('unsecured.yml')
      
      config_file = File.read(File.join(@tmp_path, 'wpa-roam.conf'))

      expect(config_file).to contain('ssid="open_network"')
      expect(config_file).to contain('key_mgmt=NONE')
      
      expect(status.success?).to be_true
    end

  end
end
