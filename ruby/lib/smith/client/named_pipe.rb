#  File: named_pipe.rb
#  Encapsulates a named pipe
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

require 'mkfifo'
require 'fcntl'

module Smith
  module Client
    class NamedPipe

      # Convenience methods for construction
      class << self
        def status_pipe
          new(Settings.status_pipe)
        end
      end

      def initialize(path)
        # path is the path to the named pipe
        @path = path

        # Ensure a named pipe located at path exists
        create
      end

      def create
        # Don't do anything if a named pipe already exists at path
        return if File.pipe?(@path)

        # If any filesystem node exists at path, delete it
        File.delete(@path) if File.exist?(@path)

        # Create the named pipe at path
        File.mkfifo(@path)
      end

      def io
        @io ||= IO.new(IO.sysopen(Settings.status_pipe, Fcntl::O_RDONLY | Fcntl::O_NONBLOCK))
      end

    end
  end
end
