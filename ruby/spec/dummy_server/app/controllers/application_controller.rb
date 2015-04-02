class ApplicationController < ActionController::Base
  # Prevent CSRF attacks by raising an exception.
  # For APIs, you may want to use :null_session instead.
  #protect_from_forgery with: :exception

  before_action :check_format, :check_auth_token

  # Requests from the test do not need to provide an authentication token in the header
  # Additionally, the create printer endpoint is public
  skip_before_action :check_auth_token, only: [:register_printer, :command, :identify, :create_printer]

  ################################################################################
  # Endpoints for printer client to communicate with directly
  ################################################################################
  
  # POST /printers
  # Printer registers itself to the server.
  # If the printer provides an authentication token, the printer will be re-associated with its account.
  # If the printer does not provide an authentication token, it will be given a new account, a new authentication token,
  # as well as a registration code it should display on the screen.
  def create_printer
    if request.headers['X-Printer-Auth-Token'] == AUTH_TOKEN
      # auth token provided and valid
      # registration code is not provided if printer has an auth token
      render json: { printer_id: PRINTER_ID, registration_code: REGISTRATION_CODE, auth_token: AUTH_TOKEN, registered: true, registration_url: REGISTRATION_URL }
    elsif !request.headers.key?('X-Printer-Auth-Token')
      # auth token not provided, return one back in the response
      render json: { printer_id: PRINTER_ID, registration_code: REGISTRATION_CODE, auth_token: AUTH_TOKEN, registered: false, registration_url: REGISTRATION_URL }
    else
      # auth token is invalid
      head :forbidden
    end
    publish_test_notification
  end

  # POST /printers/ID/acknowledge
  # Printer acknowledges receipt of command
  def command_acknowledgement
    publish_test_notification
    head :ok
  end

  # POST /printers/ID/status
  # Printer provides status update
  def status_update
    publish_test_notification
    head :ok
  end

  ################################################################################
  # Endpoints for test purposes
  ################################################################################
  # Check if the server is responsive
  # Include constants so they can be used by tests
  def identify
    render json: { printer_id: PRINTER_ID, registration_code: REGISTRATION_CODE, auth_token: AUTH_TOKEN, registration_url: REGISTRATION_URL }
  end

  # POST /users/:user_id/printers
  # User types the registration code into the web interface to register with a new printer.
  def register_printer
    if params[:registration_code] == REGISTRATION_CODE
      faye_client.publish("/printers/#{PRINTER_ID}/users", { registration: 'success', type: 'primary' }.to_json)
      head :ok
    else
      head :bad_request
    end
  end

  # POST /command
  # Test endpoint to enable tests to send commands to client over faye
  # The message_type parameter controls weather the message is sent as a JSON string or a Ruby hash
  # If the message_type parameter is not specified, the message is sent as a JSON string
  def command
    if params[:application].delete(:message_type) == 'Hash'
      message = params[:application]
    else
      message = params[:application].to_json
    end

    if params[:command]
      faye_client.publish("/printers/#{PRINTER_ID}/command", message)
      head :ok
    else
      head :bad_request
    end
  end

  private

  # Notify automated test that server received request
  def publish_test_notification
    faye_client.publish('/test', { request_params: params[:application], request_endpoint: request.original_fullpath }.to_json)
  end

  def faye_client
    FayeRails::Controller
  end

  def check_format
    head :not_acceptable unless request.format.to_s == 'application/json'
    head :unsupported_media_type unless request.content_type == 'application/json'
  end

  def check_auth_token
    head :forbidden unless request.headers['X-Printer-Auth-Token'] == AUTH_TOKEN
  end

end
