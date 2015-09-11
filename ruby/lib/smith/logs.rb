#  File: logs.rb
#  Functionality to archive log files
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#  
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'zlib'
require 'smith/tar_writer'

module Smith
  module Logs
    module_function

    # Creates archive containing logs and returns contents of the gzipped tar archive
    def get_archive
      # Glob to get log files of interest
      files = Dir[File.join(Settings.log_dir, 'syslog*')]

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
