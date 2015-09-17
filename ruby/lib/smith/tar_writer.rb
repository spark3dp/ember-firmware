#  File: tar_writer.rb
#  Custom tar writer class with additional method allowing mtime header value to be specified
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

require 'rubygems/package'

module Smith
  class TarWriter < Gem::Package::TarWriter
    def add_file_mtime(name, mode, mtime) # :yields: io
      check_closed

      raise Gem::Package::NonSeekableIO unless @io.respond_to? :pos=

      name, prefix = split_name name

      init_pos = @io.pos
      @io.write "\0" * 512 # placeholder for the header

      yield RestrictedStream.new(@io) if block_given?

      size = @io.pos - init_pos - 512

      remainder = (512 - (size % 512)) % 512
      @io.write "\0" * remainder

      final_pos = @io.pos
      @io.pos = init_pos

      header = Gem::Package::TarHeader.new :name => name, :mode => mode,
                                           :size => size, :prefix => prefix,
                                           :mtime => mtime

      @io.write header
      @io.pos = final_pos

      self
    end
  end
end
