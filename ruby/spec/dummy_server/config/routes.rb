Rails.application.routes.draw do

  post 'api/v1/print/printers/registration_code' => 'application#create_printer'
  post 'api/v1/print/printers/command/:task_id' => 'application#command_acknowledgement'
  post 'api/v1/print/printers/status' => 'application#status_update'

  # only used for communication between tests and dummy server
  get '__identify__' => 'application#identify'
  post 'v1/user/printers' => 'application#register_printer'
  post 'command' => 'application#command'

  # Endpoint that redirects to static file for testing purposes
  get 'latest_firmware', to: redirect('smith-0.0.2-valid.tar', status: 302)

end
