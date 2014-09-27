Rails.application.routes.draw do

  get '__identify__' => 'application#identify'
  get '__subscriptions__' => 'application#subscriptions'
  post 'v1/printers' => 'application#create_printer'
  post 'v1/user/printers' => 'application#register_printer'

end
