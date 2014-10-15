# Class to handle registration with web service

module Smith
  module Client
    class Registrant
      include URLHelper

      def initialize
        @state, @printer, @status_pipe = State.load, Printer.new, NamedPipe.status_pipe
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
          registration_request = Client.post_request(registration_endpoint, auth_token: @state.auth_token)
          registration_request.errback { registration_request_failed }
          registration_request.callback { |request| registration_request_successful(request.response) }
        end
      rescue Printer::InvalidState => e
        Client.log_warn("#{e.message}, retrying in #{Settings.client_retry_interval} seconds")
        EM.add_timer(Settings.client_retry_interval) { attempt_registration }
      end

      # Disconnect/cancel any open/pending connections/operations
      def disconnect
        @status_monitor.detach     if @status_monitor
        @faye_client.disconnect    if @faye_client
        @health_check_timer.cancel if @health_check_timer
        @faye_client, @status_monitor, @health_check_timer = nil, nil, nil
      end

      private

      def registration_request_failed
        Client.log_warn("Unable to reach server (#{Settings.server_url.inspect}), retrying in #{Settings.client_retry_interval} seconds")
        EM.add_timer(Settings.client_retry_interval) { attempt_registration }
      end

      def registration_request_successful(raw_response)
        # Discard any existing connections if re-establishing contact with server
        disconnect

        Client.log_info('Successfully received response from registration request')
        response = JSON.parse(raw_response, symbolize_names: true)
        Client.log_debug("Successfully parsed registration response: #{response.inspect}")
        
        @state.update(auth_token: response[:auth_token], printer_id: response[:id])
    
        @faye_client = Faye::Client.new(client_endpoint)
        @faye_client.add_extension(AuthenticationExtension.new(@state.auth_token))

        # Start periodic health checks
        @health_check_timer = EM.add_periodic_timer(Settings.client_health_check_interval) { send_health_check }

        # Don't listen for primary registration requests if registration code was not received
        # Otherwise subscribe to registration notification channel
        if response[:registration_code]
          registration_subscription =
            @faye_client.subscribe(registration_channel) { |payload| registration_notification_received(payload) }
          registration_subscription.callback { registration_notification_subscription_successful(response) }
          registration_subscription.errback { Client.log_error("Unable to subscribe to #{registration_channel}") }
        end

        # Subscribe to command notification channel
        command_subscription =
          @faye_client.subscribe(command_channel) { |payload| @command_interpreter.interpret(payload) }
        command_subscription.callback { Client.log_info("Successfully subscribed to #{command_channel}") }
        command_subscription.errback { Client.log_error("Unable to subscribe to #{command_channel}") }

        # Start watching status pipe for status updates
        @status_monitor = EM.watch(@status_pipe.io, StatusMonitor, @state)
        @status_monitor.notify_readable = true
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

      def send_health_check
        request = Client.post_request(health_check_endpoint, firmware_version: FIRMWARE_VERSION)
        request.errback { Client.log_error('Attempt to post health check failed') }
      end

    end
  end
end
