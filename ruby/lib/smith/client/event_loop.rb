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
          Signal.trap('INT') { stop }
          Signal.trap('TERM') { stop }
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
        @printer.validate_state { |state, uisubstate| state == 'Home' }
        Client.log_info("Attempting to register with server at #{registration_endpoint}")
        EM.next_tick do
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
      rescue Printer::CommunicationError => e
        Client.log_error("#{e.message}, retrying in #{@retry_interval} seconds")
        EM.add_timer(@retry_interval) { attempt_registration }
      end

      def registration_request_failed
        Client.log_warn("Unable to reach server (#{Client.server_url}), retrying in #{@retry_interval} seconds")
        EM.add_timer(@retry_interval) { attempt_registration }
      end

      def registration_request_successful(raw_response)
        Client.log_info('Successfully received response from registration request')
        response = JSON.parse(raw_response, symbolize_names: true)
        Client.log_info("Successfully parsed registration response: #{response.inspect}")
        
        @state.auth_token = response[:auth_token]
        @state.id = response[:id]
    
        @faye_client = Faye::Client.new(client_endpoint)
        @faye_client.add_extension(AuthenticationExtension.new(@state.auth_token))

        registration_subscription =
          @faye_client.subscribe(registration_channel) { |payload| registration_notification_received(payload) }
        registration_subscription.callback { registration_notification_subscription_successful(response) }
        registration_subscription.errback { Client.log_error("Unable to subscribe to #{registration_channel}") }
      end

      def registration_notification_received(payload)
        Client.log_info("Received message from server on #{registration_channel} containing #{payload}")
        @printer.send_command(Printer::Commands::PRIMARY_REGISTRATION_SUCCEEDED)
      end

      def registration_notification_subscription_successful(response)
        # TODO: remove override when server is sending registration url
        response[:registration_url] = 'autodesk.com/spark'

        Client.log_info("Successfully subscribed to #{registration_channel}")
        File.write(Client.registration_info_file,
          { registration_code: response[:registration_code], registration_url: response[:registration_url]}.to_json)
        @printer.send_command(Printer::Commands::DISPLAY_PRIMARY_REGISTRATION_CODE)
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

    end
  end
end
