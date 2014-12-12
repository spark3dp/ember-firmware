require 'smith'
require 'smith/printer'
require 'smith/config/firmware'
require 'smith/client/log_messages'
require 'smith/client/log_message'
require 'smith/client/url_helper'
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
      event_loop = EventLoop.new(State.load)
      
      # Setup signal handling
      Signal.trap('INT') { event_loop.stop }
      Signal.trap('TERM') { event_loop.stop }

      # Start the event loop
      event_loop.start
    end

  end
end
