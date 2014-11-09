module Smith
  module Client
    class HTTPClient

      def initialize(state)
        @state = state
      end

      # Set the callback that will be called when a forbidden (403) HTTP status is encountered
      def set_forbidden_callback(&block)
        @forbidden_callback = block
      end

      # Make a JSON post request to server
      # endpoint is the full url of the request
      # body is an object that can be serialized as JSON with #to_json
      # The return value is a deferred object that will call success callbacks only if
      # a response is returned and the HTTP status code indicates success
      # The deferred object will call the error callbacks if the request completes but the status code
      # indicates an error or the server at the specified endpoint cannot be reached
      # The registrant is passed in to allow re-registering if the auth token is invalid (403 error)
      def post(endpoint, body)
        deferred = EM::DefaultDeferrable.new
        body = body.to_json

        Client.log_debug(LogMessages::START_POST_REQUEST, endpoint, body)

        request = EM::HttpRequest.new(endpoint, connect_timeout: Settings.post_request_connect_timeout).post(
          head: {
            'Content-Type'       => 'application/json',
            'Accept'             => 'application/json',
            'Printer-Auth-Token' => @state.auth_token
          },
          body: body
        )
        
        request.callback do
          header = request.response_header
          if header.http_status.to_i == 403
            Client.log_error(LogMessages::POST_REQUEST_HTTP_ERROR, endpoint, body, header.status)
            deferred.fail(request)
            @forbidden_callback.call if @forbidden_callback
          elsif header.successful?
            Client.log_debug(LogMessages::POST_REQUEST_SUCCESS, endpoint, body, header.status)
            deferred.succeed(request)
          else
            Client.log_error(LogMessages::POST_REQUEST_HTTP_ERROR, endpoint, body, header.status)
            deferred.fail(request)
          end
        end
        
        request.errback do
          Client.log_error(LogMessages::POST_REQUEST_URL_UNREACHABLE, endpoint, body)
          deferred.fail(request)
        end

        deferred
      end

      # Download a file from specified url
      # Stream the contents into file
      # The return value is a deferred object that will call success callbacks only if
      # a response is returned and the HTTP status code indicates success
      # The deferred object will call the error callbacks if the request completes but the status code
      # indicates an error or the server at the specified endpoint cannot be reached
      def get_file(url, file, options = {})
        deferred = EM::DefaultDeferrable.new

        Client.log_debug(LogMessages::START_FILE_DOWNLOAD, url, file.path)

        request = EM::HttpRequest.new(url, connect_timeout: Settings.file_download_connect_timeout).get(options)

        request.callback do
          file.close
          header = request.response_header
          if header.successful?
            Client.log_debug(LogMessages::FILE_DOWNLOAD_SUCCESS, url, file.path, header.status)
            deferred.succeed(request)
          else
            Client.log_error(LogMessages::FILE_DOWNLOAD_HTTP_ERROR, url, file.path, header.status)
            deferred.fail(request)
          end
        end

        request.errback do
          file.close
          Client.log_error(LogMessages::FILE_DOWNLOAD_URL_UNREACHABLE, url)
          deferred.fail(request)
        end
        
        request.stream do |chunk|
          # Don't store the chunk if the request is in the process of being redirected
          file.write(chunk) if !request.response_header.redirection?
        end

        deferred
      end

    end
  end
end
