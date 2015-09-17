#  File: cli.rb
#  Command line interface for configuring various aspects of client
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

require 'optparse'
require 'ostruct'
require 'logger'
require 'smith/client'

module Smith
  module Client
    module CLI

      module_function

      def start(args)
        options = OpenStruct.new(
          log_level:  Logger::DEBUG,
          log_format: nil
        )

        opts = OptionParser.new do |opts|

          opts.banner = 'Usage: smith-client [options]'

          opts.on('--log-level [LEVEL]', [:debug, :info, :warn, :error, :fatal, :unknown],
                  'Specify the log level, default is debug (debug, info, warn, error, fatal, or unknown)') do |log_level|

            options.log_level = parse_log_level(log_level)

          end

          opts.on('--faye-log-level [LEVEL]', [:debug, :info, :warn, :error, :fatal, :unknown],
                  'Specify the Faye log level, default is disabled (debug, info, warn, error, fatal, or unknown)') do |log_level|

            Client.enable_faye_logging(parse_log_level(log_level), STDOUT)

          end

          opts.on('--[no-]brief-log-format', 'Format log output for syslog, default is false (suppress time, process name, etc.)') do |brief|
            
            options.log_format = Client.brief_log_format if brief

          end

        end

        opts.parse!(args)
        
        Client.enable_logging(options.log_level, STDOUT, options.log_format)
        Client.start
      end

      def parse_log_level(option)
        case option
        when :debug   then Logger::DEBUG
        when :info    then Logger::INFO
        when :warn    then Logger::WARN
        when :error   then Logger::ERROR
        when :fatal   then Logger::FATAL
        when :unknown then Logger::UNKNOWN
        else               Logger::DEBUG
        end
      end

    end
  end
end
