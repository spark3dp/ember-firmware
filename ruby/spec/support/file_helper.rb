module FileHelper

  def self.included(including_class)
    including_class.extend ClassMethods
  end

  module ClassMethods
    def wpa_roam_file_setup
      # Need to tag example group with tmp_dir to use this method
      let(:wpa_roam_file) { File.read(File.join(@tmp_path, 'wpa-roam.conf')) }
    end
  end

end
