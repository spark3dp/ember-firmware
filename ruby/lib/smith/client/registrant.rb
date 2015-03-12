# Class to handle registration with web service

module Smith
  module Client
    class Registrant

      include URLHelper

      def initialize(state, http_client)
        @state, @http_client = state, http_client
        @command_interpreter = CommandInterpreter.new(@state, @http_client)
      end

      def attempt_registration
        # Not concerned with printer state unless primary registration is occurring
        if @state.auth_token.nil?
          Client.log_info(LogMessages::BEGIN_REGISTRATION_WITHOUT_TOKEN)
          Printer.validate_state { |state, substate| state == HOME_STATE }
        else
          Client.log_info(LogMessages::BEGIN_REGISTRATION_WITH_TOKEN, @state.auth_token, @state.printer_id)
        end

        Client.log_info(LogMessages::REGISTRATION_ATTEMPT, registration_endpoint)
        registration_request = @http_client.post(registration_endpoint, type_id: Settings.spark_printer_type_id, firmware: VERSION)
        registration_request.errback { |request| registration_request_failed(request) }
        registration_request.callback { |request| registration_request_successful(request.response) }
      rescue Printer::InvalidState, Printer::CommunicationError => e
        Client.log_warn(LogMessages::RETRY_REGISTRATION_AFTER_ERROR, e.message, Settings.client_retry_interval)
        EM.add_timer(Settings.client_retry_interval) { attempt_registration }
      end

      # Disconnect/cancel any open/pending connections/operations
      # callback is called when disconnection has been processed
      def disconnect(&callback)
        @faye_client.disconnect if @faye_client
        EM.next_tick { callback.call }
      end

      # Clear the auth_token and id, disconnect the faye client, and attempt registration
      def reregister
        @state.update(printer_id: nil, auth_token: nil)
        disconnect { attempt_registration }
      end

      private

      def registration_request_failed(request)
        # If the response status is 403, the http client calls #attempt_registration immediately so don't set a timer
        if request.response_header.http_status.to_i != 403
          Client.log_warn(LogMessages::RETRY_REGISTRATION_AFTER_REQUEST_FAILED, Settings.server_url, Settings.client_retry_interval)
          EM.add_timer(Settings.client_retry_interval) { attempt_registration }
        end
      end

      def registration_request_successful(raw_response)
        Client.log_info(LogMessages::RECEIVE_REGISTRATION_RESPONSE, raw_response)
        response = JSON.parse(raw_response, symbolize_names: true)
        
        @state.update(auth_token: response[:auth_token], printer_id: response[:printer_id])
    
        @faye_client = Faye::Client.new(client_endpoint)
        @faye_client.add_extension(AuthenticationExtension.new(@state.auth_token))

        # Don't listen for primary registration requests if the printer is already registered
        # Otherwise subscribe to registration notification channel
    #    if response[:registration_code]
        if !response[:registered]
          registration_subscription =
            @faye_client.subscribe(registration_channel) { |payload| registration_notification_received(payload) }
          registration_subscription.callback { registration_notification_subscription_successful(response) }
          registration_subscription.errback { Client.log_error(LogMessages::SUBSCRIPTION_ERROR, registration_channel) }
        else
          # Query smith for the status and send it out so the server is aware of the latest status
          # This is only done if the primary registration has already been completed because
          # during primary registration, the printer state changes causing status updates that are
          # sent to the server via the StatusMonitor
          @http_client.post(status_endpoint, Printer.get_status)
        end

        # Subscribe to command notification channel
        command_subscription =
          @faye_client.subscribe(command_channel) { |payload| @command_interpreter.interpret(payload) }
        command_subscription.callback { Client.log_info(LogMessages::SUBSCRIPTION_SUCCESS, command_channel) }
        command_subscription.errback { Client.log_error(LogMessages::SUBSCRIPTION_ERROR, command_channel) }
      end

      def registration_notification_received(payload)
        Client.log_info(LogMessages::RECEIVE_NOTIFICATION, registration_channel, payload)
        Printer.send_command(CMD_REGISTERED)
        # Status update will be sent now that printer is registered with the server
      end

      def registration_notification_subscription_successful(response)
        Client.log_info(LogMessages::SUBSCRIPTION_SUCCESS, registration_channel)
        Printer.write_registration_info_file(REGISTRATION_CODE_KEY => response[:registration_code], REGISTRATION_URL_KEY => response[:registration_url])
        Printer.send_command(CMD_REGISTRATION_CODE)
      end

    end
  end
end
