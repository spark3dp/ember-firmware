# Definitions of log message templates
# Messages can be templates that get interpolated with parameters
# args is an array of the arguments passed to the log call after the message/template string
# The ERB is evaluated in the context of a LogMessage instance

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
      
      HEALTH_CHECK_ERROR = 'Health check request failed'
      
      RECEIVE_COMMAND = 'Received command message from server containing <%= args[0].inspect %>'
      ACKNOWLEDGE_COMMAND = 'Successfully acknowledged <%= args[0].inspect %> command (command token: <%= args[1].inspect %>, state: <%= args[2].inspect %>, message: <%= args[3].inspect %>)'
      
      EXCEPTION_BRIEF = '<%= args[0].message %> (<%= args[0].class %>)'
      
      PRINTER_NOT_READY_FOR_DATA = '<%= args[0] %>, not downloading print data'
      
      LOG_UPLOAD_ERROR = "Log upload failed:\n<%= format_exception(args[0]) %>"
      LOG_UPLOAD_SUCCESS = 'Successfully uploaded logs to <%= args[0].inspect %>'

      PRINT_ENGINE_COMMAND_ERROR = "Error sending print engine command:\n<%= format_exception(args[0]) %>"
      PRINT_ENGINE_COMMAND_SUCCESS = 'Successfully sent print engine command: <%= args[0].inspect %>'
      
      FIRMWARE_DOWNLOAD_ERROR = 'Error downloading firmware package from <%= args[0].inspect %>'
      FIRMWARE_DOWNLOAD_SUCCESS = 'Firmware package download from <%= args[0].inspect %> complete, file downloaded to <%= args[1].inspect %>'
      FIRMWARE_UPGRADE_ERROR = "Error upgrading firmware:\n<%= format_exception(args[0]) %>"
      FIRMWARE_UPGRADE_SUCCESS = 'Firmware upgrade successfully'

      PRINT_DATA_DOWNLOAD_ERROR = 'Error downloading print data from <%= args[0].inspect %>'
      PRINT_DATA_DOWNLOAD_SUCCESS = 'Print data download from <%= args[0].inspect %> complete, file downloaded to <%= args[1].inspect %>'
      PRINT_DATA_LOAD_ERROR = "Error loading print data:\n<%= format_exception(args[0]) %>"
      PRINT_DATA_LOAD_SUCCESS = 'Print data loaded successfully'

      START_POST_REQUEST = 'Starting post request to <%= args[0].inspect %> with body <%= args[1].inspect %>'
      POST_REQUEST_SUCCESS = 'Post request to <%= args[0].inspect %> with body <%= args[1].inspect %> successful, got HTTP status code <%= args[2] %>'
      POST_REQUEST_HTTP_ERROR = 'Post request to <%= args[0].inspect %> with body <%= args[1].inspect %> unsuccessful, got HTTP status code <%= args[2] %>'
      POST_REQUEST_URL_UNREACHABLE = 'Unable to reach <%= args[0].inspect %> via post request (body: <%= args[1].inspect %>)'

      START_FILE_DOWNLOAD = 'Starting file download from <%= args[0].inspect %> to <%= args[1].inspect %>'
      FILE_DOWNLOAD_SUCCESS = 'File download from <%= args[0].inspect %> to <%= args[1].inspect %> successful, got HTTP status code <%= args[2] %>'
      FILE_DOWNLOAD_HTTP_ERROR = 'File download from <%= args[0].inspect %> to <%= args[1].inspect %> unsuccessful, got HTTP status code <%= args[2] %>'
      FILE_DOWNLOAD_URL_UNREACHABLE = 'Unable to reach <%= args[0].inspect %> to download file'

      RECEIVE_STATUS_UPDATE = 'Received status update: <%= args[0].inspect %>'

      START_EVENT_LOOP = 'Starting event loop'
      STOP_EVENT_LOOP = 'Stopping event loop'
    end
  end
end
