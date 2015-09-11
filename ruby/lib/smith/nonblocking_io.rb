#  File: nonblocking_io.rb
#  A non-blocking version of IO#readlines by wrapping IO objects
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

module Smith
  class NonblockingIO

    def initialize(io)
      @io = io
    end

    # Similiar to IO#readlines but will return empty array
    # rather than blocking if there is no data to read or
    # the EOF is not reached
    def readlines_nonblock
      lines = []
      buffer = ''
      loop do
        buffer << @io.read_nonblock(1)
        if buffer[-1] == "\n"
          lines << buffer
          buffer = ''
        end
      end
    rescue Errno::EAGAIN, EOFError
      return lines
    end

  end
end
