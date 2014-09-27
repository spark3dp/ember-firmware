class ApplicationController < ActionController::Base
  # Prevent CSRF attacks by raising an exception.
  # For APIs, you may want to use :null_session instead.
  protect_from_forgery with: :exception

  # Check if the server is responsive
  def identify
    head 200
  end

  # Check nubmer of completed faye subscriptions for a given faye client_id
  def subscriptions
    render json: { subscription_count: FAYE_SUBSCRIPTIONS[params[:faye_client_id]] }
  end

  # POST /printers
  # Printer registers itself to the server.
  # If the printer provides an authentication token, the printer will be re-associated with its account.
  # If the printer does not provide an authentication token, it will be given a new account, a new authentication token,
  # as well as a registration code it should display on the screen.
  def create_printer
    render json: { id: 539, registration_code: '4321', auth_token: 'authtoken', registration_url: 'autodesk.com/spark' }
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

  private

  def faye_client
    FayeRails::Controller
  end

end
