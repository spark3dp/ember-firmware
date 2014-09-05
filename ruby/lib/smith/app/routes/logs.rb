require 'zlib'
require 'smith/tar_writer'

module Smith
  module App
    class Application < Sinatra::Base
      
      get '/logs' do
        headers['Content-Disposition'] = "attachment;filename=smith-#{Smith::VERSION}_logs_#{Time.now.strftime('%m-%d-%Y')}.tar.gz"
        files = Dir[File.join(Application.log_dir, 'smith*')] + Dir[File.join(Application.log_dir, 'syslog*')]
        tar_io = StringIO.new
        Smith::TarWriter.new(tar_io) do |t|
          files.each do |file|
            t.add_file_mtime(File.basename(file), 0644, File.mtime(file)) { |tf| tf.write(File.read(file)) }
          end
        end
        tar_io.rewind
        gz_io = StringIO.new
        gz = Zlib::GzipWriter.new(gz_io)
        gz.write(tar_io.read)
        gz.close
        gz_io.string
      end

    end
  end
end
