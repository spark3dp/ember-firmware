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

    def enable_logging(logdev = nil, level = Logger::DEBUG, formatter = nil)
      # Flush stdout immediately if using STDOUT as logdev
      STDOUT.sync = true if logdev.nil?
      self.logger = Logger.new(logdev || STDOUT)
      logger.progname = 'smith-client'
      logger.formatter = formatter if formatter
      logger.level = level
    end

    def enable_faye_logging(logdev = nil)
      # Flush stdout immediately if using STDOUT as logdev
      STDOUT.sync = true if logdev.nil?
      Faye.logger = Logger.new(logdev || STDOUT)
      Faye.logger.level = Logger::INFO
    end

    def start
      event_loop = EventLoop.new
      
      # Setup signal handling
      Signal.trap('INT') { event_loop.stop }
      Signal.trap('TERM') { event_loop.stop }

      # Start the event loop
      event_loop.start
    end

  end
end
