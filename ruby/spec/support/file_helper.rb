module FileHelper

  require 'smith/settings'

  def self.included(including_class)
    including_class.class_exec do
      let(:wpa_roam_file_contents) { File.read(Smith::Settings.wpa_roam_file) }
      let(:hostapd_config_file_contents) { File.read(Smith::Settings.hostapd_config_file) }
      let(:dnsmasq_config_file_contents) { File.read(Smith::Settings.dnsmasq_config_file) }
    end
  end

  def make_tmp_dir
    @tmp_dir_path = File.expand_path("#{Dir.tmpdir}/#{Time.now.to_i}#{rand(1000)}/")
    FileUtils.mkdir_p(@tmp_dir_path)
    Smith::Settings.wpa_roam_file = tmp_dir('wpa-roam.conf')
    Smith::Settings.hostapd_config_file = tmp_dir('hostapd.conf')
    Smith::Settings.dnsmasq_config_file = tmp_dir('dnsmasq.conf')
    Smith::Settings.log_dir = @tmp_dir_path
  end

  def remove_tmp_dir
    FileUtils.rm_rf(@tmp_dir_path) if File.exists?(@tmp_dir_path)
  end

  def resource(file_name)
    File.join(Smith.root, 'spec/resource', file_name)
  end

  def tmp_dir(file_name)
    File.join(@tmp_dir_path, file_name)
  end

end
