#  File: log_messages.rb
#  Definitions of log message templates
#  Messages can be templates that get interpolated with parameters
#  args is an array of the arguments passed to the log call after the message/template string
#  The ERB is evaluated in the context of a LogMessage instance
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

module Smith
  module Client
    module LogMessages
      BEGIN_REGISTRATION_WITH_TOKEN = 'Beginning registration (auth token: <%= args[0].inspect %>, ID: <%= args[1] %>)'
      BEGIN_REGISTRATION_WITHOUT_TOKEN = 'Beginning registration without auth token/ID'
      
      REGISTRATION_ATTEMPT = 'Attempting to register with server at <%= args[0].inspect %>'
      RETRY_REGISTRATION_AFTER_ERROR = '<%= args[0] %>, retrying registration in <%= args[1] %> seconds'
      RETRY_REGISTRATION_AFTER_REQUEST_FAILED = 'Unable to reach server at <%= args[0].inspect %>, retrying in <%= args[1] %> seconds'
      RECEIVE_REGISTRATION_RESPONSE = 'Successfully received response from registration request: <%= args[0].inspect %>'
      
      SUBSCRIPTION_ERROR = 'Unable to subscribe to <%= args[0].inspect %>' 
      SUBSCRIPTION_SUCCESS = 'Successfully subscribed to <%= args[0].inspect %>'
      RECEIVE_NOTIFICATION = 'Received message from server on <%= args[0].inspect %> containing <%= args[1].inspect %>'
      
      RECEIVE_COMMAND = 'Received command message from server containing <%= args[0].inspect %>'
      ACKNOWLEDGE_COMMAND = 'Successfully acknowledged <%= args[0].inspect %> command (command token: <%= args[1].inspect %>, state: <%= args[2].inspect %>, message: <%= args[3].inspect %>)'
      
      EXCEPTION_BRIEF = '<%= args[0].message %> (<%= args[0].class %>)'
      
      PRINTER_NOT_READY_FOR_DATA = '<%= args[0] %>, not downloading print data'
     
      START_LOG_UPLOAD = 'Starting log archive upload via put request to <%= args[0].inspect %>'
      LOG_UPLOAD_HTTP_ERROR = 'Log archive upload via put request to <%= args[0].inspect %> unsuccessful, got HTTP status code <%= args[1] %>'
      LOG_UPLOAD_URL_UNREACHABLE = 'Unable to reach <%= args[0].inspect %> via put request for log archive upload'
      LOG_UPLOAD_SUCCESS = 'Successfully uploaded log archive to <%= args[0].inspect %>'

      PRINT_ENGINE_COMMAND_ERROR = "Error sending print engine command:\n<%= format_exception(args[0]) %>"
      PRINT_ENGINE_COMMAND_SUCCESS = 'Successfully sent print engine command: <%= args[0].inspect %>'
      
      FIRMWARE_DOWNLOAD_ERROR = 'Error downloading firmware package from <%= args[0].inspect %>'
      FIRMWARE_DOWNLOAD_SUCCESS = 'Firmware package download from <%= args[0].inspect %> complete, file downloaded to <%= args[1].inspect %>'
      FIRMWARE_UPGRADE_ERROR = "Error upgrading firmware:\n<%= format_exception(args[0]) %>"
      FIRMWARE_UPGRADE_SUCCESS = 'Firmware upgraded successfully'

      PRINT_DATA_DOWNLOAD_ERROR = 'Error downloading print data from <%= args[0].inspect %>'
      PRINT_DATA_DOWNLOAD_SUCCESS = 'Print data download from <%= args[0].inspect %> complete, file downloaded to <%= args[1].inspect %>'
      PRINT_DATA_LOAD_ERROR = "Error loading print data:\n<%= format_exception(args[0]) %>"
      PRINT_DATA_LOAD_SUCCESS = 'Print data loaded successfully'
      PRINT_DATA_LOAD_FILE_CURRENTLY_LOADED = 'Print file specified in command (<%= args[0].inspect %>) same as currently loaded file (<%= args[1].inspect %>), applying settings'
      PRINT_DATA_LOAD_FILE_NOT_CURRENTLY_LOADED = 'Print file specified in command (<%= args[0].inspect %>) different from currently loaded file (<%= args[1].inspect %>), downloading file'

      SETTINGS_ERROR = "Error loading settings: <%= args[0].inspect %>"
      REQUEST_SETTINGS_ERROR = "Error retrieving settings: <%= args[0].inspect %>"

      START_POST_REQUEST = 'Starting post request to <%= args[0].inspect %> with body <%= args[1].inspect %>'
      POST_REQUEST_SUCCESS = 'Post request to <%= args[0].inspect %> with body <%= args[1].inspect %> successful, got HTTP status code <%= args[2] %>'
      POST_REQUEST_HTTP_ERROR = 'Post request to <%= args[0].inspect %> with body <%= args[1].inspect %> unsuccessful, got HTTP status code <%= args[2] %>'
      POST_REQUEST_URL_UNREACHABLE = 'Unable to reach <%= args[0].inspect %> via post request with body <%= args[1].inspect %>'

      START_FILE_DOWNLOAD = 'Starting file download from <%= args[0].inspect %> to <%= args[1].inspect %>'
      FILE_DOWNLOAD_SUCCESS = 'File download from <%= args[0].inspect %> to <%= args[1].inspect %> successful, got HTTP status code <%= args[2] %>'
      FILE_DOWNLOAD_HTTP_ERROR = 'File download from <%= args[0].inspect %> to <%= args[1].inspect %> unsuccessful, got HTTP status code <%= args[2] %>'
      FILE_DOWNLOAD_URL_UNREACHABLE = 'Unable to reach <%= args[0].inspect %> to download file'

      RECEIVE_STATUS_UPDATE = 'Received status update: <%= args[0].inspect %>'

      START_EVENT_LOOP = 'Starting event loop'
      STOP_EVENT_LOOP = 'Stopping event loop'

      VERSION = 'smith-client version <%= args[0] %>'

      HTTP_REQUEST_LOGGING_SUSPENSION = 'Unable to reach <%= args[0].inspect %> via post request, suspending HTTP post request error logging'
      HTTP_REQUEST_LOGGING_RESUMPTION = 'Resuming HTTP post request error logging'
    end
  end
end
