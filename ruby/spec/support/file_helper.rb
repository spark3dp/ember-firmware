module FileHelper

  def self.included(including_class)
    including_class.class_exec do
      extend ClassMethods
    end
  end

  def make_tmp_dir
    require 'tmpdir'
    @tmp_dir_path = File.expand_path("#{Dir.tmpdir}/#{Time.now.to_i}#{rand(1000)}/")
    FileUtils.mkdir_p(@tmp_dir_path)
    ENV['WPA_ROAM_DIR'] = @tmp_dir_path
    ENV['HOSTAPD_CONF_DIR'] = @tmp_dir_path
    ENV['DNSMASQ_CONF_DIR'] = @tmp_dir_path
    ENV['STORAGE_DIR'] = @tmp_dir_path
    ENV['LOG_DIR'] = @tmp_dir_path
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

  module ClassMethods
    # TODO no need for a method, just call #let on the including class
    def wpa_roam_file_setup
      # Need to tag example group with tmp_dir to use this method
      let(:wpa_roam_file) { File.read(tmp_dir('wpa-roam.conf')) }
    end
  end

end
