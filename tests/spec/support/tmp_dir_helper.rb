module Tests
  module TmpDirHelper
    def make_tmp_dir
      @tmp_dir_path = File.expand_path("#{Dir.tmpdir}/#{Time.now.to_i}#{rand(1000)}/")
      FileUtils.mkdir_p(@tmp_dir_path)
    end

    def remove_tmp_dir
      FileUtils.rm_rf(@tmp_dir_path) if File.exists?(@tmp_dir_path)
    end

    def tmp_dir(file_name)
      File.join(@tmp_dir_path, file_name)
    end

    def tmp_dir_path
      @tmp_dir_path
    end
  end
end