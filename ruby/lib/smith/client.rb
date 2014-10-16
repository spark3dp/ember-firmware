require 'smith'
require 'smith/printer'
require 'smith/client/url_helper'
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

    def log_info(message)
      logger.info(message) if logger
    end

    def log_error(message)
      logger.error(message) if logger
    end

    def log_debug(message)
      logger.debug(message) if logger
    end

    def log_warn(message)
      logger.warn(message) if logger
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
      EventLoop.new.start
    end

    # Make a JSON post request to server
    # endpoint is the full url of the request
    # body is a Hash or other object that can be serialized as JSON with #to_json
    # The return value is a deferred object that will call success callbacks only if
    # a response is returned and the HTTP status code indicates success
    # The deferred object will call the error callbacks if the request completes but the status code
    # indicates an error or the server at the specified endpoint cannont be reached
    def post_request(endpoint, body)
      deferred = EM::DefaultDeferrable.new
      body = body.to_json
      
      request = EM::HttpRequest.new(endpoint).post(
        head: { 'Content-Type' => 'application/json', 'Accept' => 'application/json' },
        body: body
      )
      
      request.callback do
        header = request.response_header
        if header.successful?
          deferred.succeed(request)
          Client.log_debug("Post request to #{endpoint.inspect} containing #{body.inspect} successful, got HTTP status code #{header.status}")
        else
          deferred.fail(request)
          Client.log_debug("Post request to #{endpoint.inspect} containing #{body.inspect} unsuccessful, got HTTP status code #{header.status}")
        end
      end
      
      request.errback do
        deferred.fail(request)
        Client.log_debug("Unable to reach server via post request to #{endpoint.inspect} (body: #{body.inspect})")
      end

      deferred
    end

  end
end
