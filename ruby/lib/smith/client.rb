#  File: client.rb
#  Top level module definition for client
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

require 'smith'
require 'smith/printer'
require 'smith/config/firmware'
require 'smith/client/log_messages'
require 'smith/client/log_message'
require 'smith/client/url_helper'
require 'smith/client/payload_helper'
require 'smith/client/http_client'
require 'smith/client/command'
require 'logger'
require 'faye'
require 'json'
require 'em-http'

Dir["#{Smith.root}/lib/smith/client/**/*.rb"].each { |f| require(f) }

module Smith
  module Client

    class << self
      attr_accessor :logger
    end

    module_function

    def log_info(*args)
      logger.info(LogMessage.format(*args)) if logger
    end

    def log_error(*args)
      logger.error(LogMessage.format(*args)) if logger
    end

    def log_debug(*args)
      logger.debug(LogMessage.format(*args)) if logger
    end

    def log_warn(*args)
      logger.warn(LogMessage.format(*args)) if logger
    end

    def enable_logging(level = Logger::DEBUG, logdev = STDOUT, formatter = nil)
      # Flush stdout immediately if using STDOUT as logdev
      STDOUT.sync = true if logdev == STDOUT
      self.logger = Logger.new(logdev)
      logger.progname = 'smith-client'
      logger.formatter = formatter if formatter
      logger.level = level
    end

    def enable_faye_logging(level = Logger::INFO, logdev = STDOUT)
      # Flush stdout immediately if using STDOUT as logdev
      STDOUT.sync = true if logdev == STDOUT
      Faye.logger = Logger.new(logdev)
      Faye.logger.level = level
    end

    def brief_log_format
      proc do |severity, datetime, progname, msg|
        "#{msg}\n"
      end
    end

    def start
      Client.log_info(LogMessages::VERSION, VERSION)

      event_loop = EventLoop.new(State.load)
      
      # Setup signal handling
      Signal.trap('INT') { event_loop.stop }
      Signal.trap('TERM') { event_loop.stop }

      # Start the event loop
      event_loop.start
    end

  end
end
