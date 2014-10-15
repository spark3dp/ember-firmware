class ApplicationController < ActionController::Base
  # Prevent CSRF attacks by raising an exception.
  # For APIs, you may want to use :null_session instead.
  #protect_from_forgery with: :exception

  before_filter :check_format

  # Check if the server is responsive
  def identify
    head 200
  end

  # POST /printers
  # Printer registers itself to the server.
  # If the printer provides an authentication token, the printer will be re-associated with its account.
  # If the printer does not provide an authentication token, it will be given a new account, a new authentication token,
  # as well as a registration code it should display on the screen.
  def create_printer
    if params[:auth_token]
      # registration code is not provided if printer has an auth token
      render json: { id: 539, registration_code: nil, auth_token: 'authtoken', registration_url: 'autodesk.com/spark' }
    else
      render json: { id: 539, registration_code: '4321', auth_token: 'authtoken', registration_url: 'autodesk.com/spark' }
    end
  end

  # POST /users/:user_id/printers
  # User types the registration code into the web interface to register with a new printer.
  def register_printer
    if params[:registration_code] == '4321'
      faye_client.publish('/printers/539/users', { registration: 'success', type: 'primary' }.to_json)
      head :ok
    else
      head :bad_request
    end
  end

  # POST /printers/ID/acknowledge
  # Printer acknowledges receipt of command
  def command_acknowledgement
    publish_test_notification
    head :ok
  end

  # POST /command
  # Test endpoint to enable tests to send commands to client over faye
  def command
    if params[:command]
      faye_client.publish('/printers/539/command', params[:application].to_json)
      head :ok
    else
      head :bad_request
    end
  end

  # POST /printers/ID/status
  # Printer provides status update
  def status_update
    publish_test_notification
    head :ok
  end

  # POST /printers/ID/health_check
  def health_check
    publish_test_notification
    head :ok
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

end
