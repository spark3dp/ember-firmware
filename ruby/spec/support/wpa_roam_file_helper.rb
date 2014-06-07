module WpaRoamFileHelper

  def wpa_roam_file_setup
    # Need to tag example group with tmp_dir to use this method
    before do
      ENV['WPA_ROAM_PATH'] = @tmp_path
      allow(Smith::Config::Wired).to receive(:connected?).and_return(false)
    end

    let(:wpa_roam_file) { File.read(File.join(@tmp_path, 'wpa-roam.conf')) }
  end

end
