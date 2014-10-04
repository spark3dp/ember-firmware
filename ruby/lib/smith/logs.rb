require 'zlib'
require 'smith/tar_writer'

module Smith
  module Logs
    module_function

    # Creates archive containing logs and returns contents of the gzipped tar archive
    def get_archive
      # Glob to get log files of interest
      files = Dir[File.join(Smith.log_dir, 'smith*')] + Dir[File.join(Smith.log_dir, 'syslog*')]

      # Create a tar file backed by a string and add the log files
      tar_io = StringIO.new
      Smith::TarWriter.new(tar_io) do |t|
        files.each do |file|
          t.add_file_mtime(File.basename(file), 0644, File.mtime(file)) { |tf| tf.write(File.read(file)) }
        end
      end
      tar_io.rewind

      # Compress the tar file
      gz_io = StringIO.new
      gz = Zlib::GzipWriter.new(gz_io)
      gz.write(tar_io.read)
      gz.close

      # Return contents as string
      # Return value can be written to a file, returned in HTTP reponse, etc.
      gz_io.string
    end

  end
end
