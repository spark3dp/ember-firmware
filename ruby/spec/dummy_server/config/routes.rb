Rails.application.routes.draw do

  get '__identify__' => 'application#identify'
  post 'v1/printers' => 'application#create_printer'
  post 'v1/printers/:printer_id/acknowledge' => 'application#command_acknowledgement'
  post 'v1/user/printers' => 'application#register_printer'
  post 'command' => 'application#command'

end
