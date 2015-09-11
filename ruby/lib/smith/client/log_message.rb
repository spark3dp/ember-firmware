#  File: log_message.rb
#  Formats log message strings given an ERB template and parameters
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

require 'erb'

module Smith
  module Client
    class LogMessage

      class << self
        def format(*args)
          new(*args).format
        end
      end

      attr_reader :args

      def initialize(template, *args)
        @template, @args = template, args
      end

      # Format an exception in a manner similar to how ruby formats them
      def format_exception(e)
        "#{e.backtrace.first}: #{e.message} (#{e.class})\n#{e.backtrace.drop(1).map { |l| "\t#{l}" }.join("\n")}"
      end

      def format
        ERB.new(@template).result(binding)
      end

    end
  end
end
