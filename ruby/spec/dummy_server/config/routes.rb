Rails.application.routes.draw do

  get '__identify__' => 'application#identify'
  post 'v1/printers' => 'application#create_printer'
  post 'v1/printers/:printer_id/acknowledge' => 'application#command_acknowledgement'
  post 'v1/printers/:printer_id/status' => 'application#status_update'
  post 'v1/printers/:printer_id/health_check' => 'application#health_check'
  post 'v1/user/printers' => 'application#register_printer'
  post 'command' => 'application#command'

  # Endpoint that redirects to static file for testing purposes
  get 'redirect', to: redirect('/test_firmware_upgrade_package', status: 302)

end
