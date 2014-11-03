module Smith
  module Client
    module RequestHelper

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

        request = EM::HttpRequest.new(endpoint, connect_timeout: 10).post(
          head: {
            'Content-Type' => 'application/json',
            'Accept' => 'application/json',
            'Printer-Auth-Token' => @state.auth_token
          },
          body: body
        )
        
        request.callback do
          header = request.response_header
          if header.successful?
            deferred.succeed(request)
            Client.log_debug("Post request to #{endpoint.inspect} containing #{body.inspect} successful, got HTTP status code #{header.status}")
          else
            deferred.fail(request)
            Client.log_error("Post request to #{endpoint.inspect} containing #{body.inspect} unsuccessful, got HTTP status code #{header.status}")
          end
        end
        
        request.errback do
          deferred.fail(request)
          Client.log_error("Unable to reach server via post request to #{endpoint.inspect} (body: #{body.inspect})")
        end

        deferred
      end

      # Download a file from specified url
      # Stream the contents into file
      # The return value is a deferred object that will call success callbacks only if
      # a response is returned and the HTTP status code indicates success
      # The deferred object will call the error callbacks if the request completes but the status code
      # indicates an error or the server at the specified endpoint cannont be reached
      def download_file(url, file, options = {})
        deferred = EM::DefaultDeferrable.new

        request = EM::HttpRequest.new(url, connect_timeout: 25).get(options)

        request.callback do
          file.close
          header = request.response_header
          if header.successful?
            deferred.succeed(request)
            Client.log_debug("File download from #{url.inspect} successful, got HTTP status code #{header.status}")
          else
            deferred.fail(request)
            Client.log_error("File download from #{url.inspect} unsuccessful, got HTTP status code #{header.status}")
          end
        end

        request.errback do
          file.close
          deferred.fail(request)
          Client.log_error("Unable to reach #{url.inspect} to download file")
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
