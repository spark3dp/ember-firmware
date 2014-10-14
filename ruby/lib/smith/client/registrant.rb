# Class to handle registration with web service

require 'json'
require 'smith/printer'

module Smith
  module Client
    class Registrant
      include URLHelper

      def initialize(state, retry_interval)
        @state, @retry_interval, @printer = state, retry_interval, Printer.new
        @command_interpreter = CommandInterpreter.new(@printer, @state)
      end

      def attempt_registration
        # Not concerned with printer state unless primary registration is occuring
        if @state.auth_token.nil?
          Client.log_debug('Validating printer state')
          @printer.validate_state { |state, substate| state == HOME_STATE }
        else
          Client.log_info("Re-establishing communication with server using auth token #{@state.auth_token.inspect}")
        end

        EM.next_tick do
          Client.log_info("Attempting to register with server at #{registration_endpoint.inspect}")
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

      def disconnect_client
        @faye_client.disconnect if @faye_client
      end

      private

      def registration_request_failed
        Client.log_warn("Unable to reach server (#{Settings.server_url.inspect}), retrying in #{@retry_interval} seconds")
        EM.add_timer(@retry_interval) { attempt_registration }
      end

      def registration_request_successful(raw_response)
        Client.log_info('Successfully received response from registration request')
        response = JSON.parse(raw_response, symbolize_names: true)
        Client.log_debug("Successfully parsed registration response: #{response.inspect}")
        
        @state.update(auth_token: response[:auth_token], printer_id: response[:id])
    
        @faye_client = Faye::Client.new(client_endpoint)
        @faye_client.add_extension(AuthenticationExtension.new(@state.auth_token))

        # Don't listen for primary registration requests if registration code was not received
        if response[:registration_code]
          registration_subscription =
            @faye_client.subscribe(registration_channel) { |payload| registration_notification_received(payload) }
          registration_subscription.callback { registration_notification_subscription_successful(response) }
          registration_subscription.errback { Client.log_error("Unable to subscribe to #{registration_channel}") }
        end

        command_subscription =
          @faye_client.subscribe(command_channel) { |payload| @command_interpreter.interpret(payload) }
        command_subscription.callback { Client.log_info("Successfully subscribed to #{command_channel}") }
        command_subscription.errback { Client.log_error("Unable to subscribe to #{command_channel}") }
      end

      def registration_notification_received(payload)
        Client.log_info("Received message from server on #{registration_channel.inspect} containing #{payload.inspect}")
        @printer.send_command(CMD_REGISTERED)
      end

      def registration_notification_subscription_successful(response)
        # TODO: remove override when server is sending registration url
        response[:registration_url] = 'autodesk.com/spark'

        Client.log_info("Successfully subscribed to #{registration_channel.inspect}")
        File.write(Settings.registration_info_file,
          { REGISTRATION_CODE_KEY => response[:registration_code], REGISTRATION_URL_KEY => response[:registration_url]}.to_json)
        @printer.send_command(CMD_REGISTRATION_CODE)
      end

    end
  end
end
