require 'smith/version'
require 'smith/definitions'

# Config options can be set through environment variables
# These are the defaults if not set
# TODO: Document these options
ENV['WPA_ROAM_DIR']           ||= '/var/local'
ENV['HOSTAPD_CONF_DIR']       ||= '/var/local'
ENV['DNSMASQ_CONF_DIR']       ||= '/var/local'
ENV['STORAGE_DIR']            ||= '/var/local'
ENV['COMMAND_PIPE']           ||= Smith::COMMAND_PIPE
ENV['COMMAND_RESPONSE_PIPE']  ||= Smith::COMMAND_RESPONSE_PIPE
ENV['PRINT_DATA_DIR']         ||= '/var/smith/download'
ENV['FIRMWARE_DIR']           ||= '/main/firmware'
ENV['WIRELESS_INTERFACE']     ||= 'wlan0'
ENV['WIRED_INTERFACE']        ||= 'eth0'
ENV['AP_SSID_PREFIX']         ||= 'Spark WiFi'
ENV['AP_IP']                  ||= '192.168.1.1/24'
ENV['LOG_DIR']                ||= '/var/log'
ENV['SERVER_URL']             ||= 'http://printer-backend-dev.ngrok.com'
ENV['CLIENT_RETRY_INTERVAL']  ||= '60'
ENV['REGISTRATION_INFO_FILE'] ||= '/tmp/printer_registration'
ENV['PRINT_SETTINGS_FILE']    ||= '/tmp/printsettings'
ENV['S3_LOG_BUCKET']          ||= 'ember-log-archives'

# These are the credentials for the AWS user "ember_printer"
ENV['AWS_ACCESS_KEY_ID']      ||= 'AKIAIEEHFUR53SNDSFMA'
ENV['AWS_SECRET_ACCESS_KEY']  ||= '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'

module Smith
  module_function

  def root
    @root ||= File.expand_path('../..', __FILE__)
  end

  def command_pipe
    ENV['COMMAND_PIPE']
  end

  def command_response_pipe
    ENV['COMMAND_RESPONSE_PIPE']
  end

  def print_data_dir
    ENV['PRINT_DATA_DIR']
  end

  def log_dir
    ENV['LOG_DIR']
  end
end

