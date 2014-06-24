module FileHelper

  def self.included(including_class)
    including_class.extend ClassMethods
  end

  def resource(file_name)
    File.join(Smith.root, 'spec/resource', file_name)
  end

  def tmp_dir(file_name)
    File.join(@tmp_dir_path, file_name)
  end

  module ClassMethods
    def wpa_roam_file_setup
      # Need to tag example group with tmp_dir to use this method
      let(:wpa_roam_file) { File.read(tmp_dir('wpa-roam.conf')) }
    end
  end

end
