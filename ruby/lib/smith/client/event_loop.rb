require 'smith/printer'
require 'json'
require 'fcntl'

module Smith
  module Client
    class EventLoop

      def initialize(printer_state, retry_interval)
        @state, @retry_interval = printer_state, retry_interval
        @printer = Printer.new
      end

      def start
        Client.log_info('Starting event loop')
        EM.run do
          # Setup signal handling
          Signal.trap('INT') { stop }
          Signal.trap('TERM') { stop }
          # Attempt to contact the server
          attempt_registration 
        end
      end

      def stop
        Client.log_info('Stopping event loop')
        EM.next_tick do
          @faye_client.disconnect if @faye_client
          # Need to allow the event loop to process the disconnect before stopping
          EM.next_tick { EM.stop }
        end
      end

      private

      def attempt_registration
        Client.log_debug('Validating printer state')
        @printer.validate_state { |state, substate| state == HOME_STATE }
        EM.next_tick do
          Client.log_info("Attempting to register with server at #{registration_endpoint}")
          registration_request = EM::HttpRequest.new(registration_endpoint).post(
            head: { 'Content-Type' => 'application/json' },
            body: { auth_token: @state.auth_token }.to_json
          )
          registration_request.errback { registration_request_failed }
          registration_request.callback { registration_request_successful(registration_request.response) }
        end
      rescue Printer::InvalidState => e
        Client.log_warn("#{e.message}, retrying in #{@retry_interval} seconds")
        EM.add_timer(@retry_interval) { attempt_registration }
      end

      def registration_request_failed
        Client.log_warn("Unable to reach server (#{Client.server_url}), retrying in #{@retry_interval} seconds")
        EM.add_timer(@retry_interval) { attempt_registration }
      end

      def registration_request_successful(raw_response)
        Client.log_info('Successfully received response from registration request')
        response = JSON.parse(raw_response, symbolize_names: true)
        Client.log_debug("Successfully parsed registration response: #{response.inspect}")
        
        @state.auth_token = response[:auth_token]
        @state.id = response[:id]
    
        @faye_client = Faye::Client.new(client_endpoint)
        @faye_client.add_extension(AuthenticationExtension.new(@state.auth_token))

        registration_subscription =
          @faye_client.subscribe(registration_channel) { |payload| registration_notification_received(payload) }
        registration_subscription.callback { registration_notification_subscription_successful(response) }
        registration_subscription.errback { Client.log_error("Unable to subscribe to #{registration_channel}") }

        command_subscription =
          @faye_client.subscribe(command_channel) { |payload| command_received(payload) }
        command_subscription.callback { Client.log_info("Successfully subscribed to #{command_channel}") }
        command_subscription.errback { Client.log_error("Unable to subscribe to #{command_channel}") }
      end

      def registration_notification_received(payload)
        Client.log_info("Received message from server on #{registration_channel} containing #{payload}")
        @printer.send_command(CMD_REGISTERED)
      end

      def registration_notification_subscription_successful(response)
        # TODO: remove override when server is sending registration url
        response[:registration_url] = 'autodesk.com/spark'

        Client.log_info("Successfully subscribed to #{registration_channel}")
        File.write(Client.registration_info_file,
          { REGISTRATION_CODE_KEY => response[:registration_code], REGISTRATION_URL_KEY => response[:registration_url]}.to_json)
        @printer.send_command(CMD_REGISTRATION_CODE)
      end

      def command_received(raw_payload)
        Client.log_info("Received message from server on #{command_channel} containing #{raw_payload}")
        payload = JSON.parse(raw_payload, symbolize_names: true)
        Client.log_debug("Successfully parsed command notification payload: #{payload.inspect}")
        if payload[:command] == 'print_data'
          PrintDataCommand.new(@printer, payload).handle
        elsif payload[:command] == 'logs'
          LogsCommand.new(@printer, payload, @state.id).handle
        else
          PrintEngineCommand.new(@printer, payload).handle
        end
      end

      def client_endpoint
        "#{Client.server_url}/faye"
      end

      def registration_endpoint
        "#{Client.server_url}/#{Client.api_version}/printers"
      end

      def registration_channel
        "/printers/#{@state.id}/users"
      end

      def command_channel
        "/printers/#{@state.id}/command"
      end
    end
  end
end
