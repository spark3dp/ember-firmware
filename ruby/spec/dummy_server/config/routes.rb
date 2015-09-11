#  File: routes.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

  # Endpoint for testing file uploads
  put 'test_upload' => 'application#file_upload'
end
