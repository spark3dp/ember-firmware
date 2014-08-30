require 'smith/version'
require 'smith/definitions'

# Config options can be set through environment variables
# These are the defaults if not set
ENV['WPA_ROAM_DIR']          ||= '/var/local'
ENV['HOSTAPD_CONF_DIR']      ||= '/var/local'
ENV['DNSMASQ_CONF_DIR']      ||= '/var/local'
ENV['STORAGE_DIR']           ||= '/var/local'
ENV['COMMAND_PIPE']          ||= Smith::COMMAND_PIPE
ENV['COMMAND_RESPONSE_PIPE'] ||= Smith::COMMAND_RESPONSE_PIPE
ENV['UPLOAD_DIR']            ||= '/var/smith/download'
ENV['FIRMWARE_DIR']          ||= '/main/firmware'
ENV['WIRELESS_INTERFACE']    ||= 'wlan0'
ENV['WIRED_INTERFACE']       ||= 'eth0'
ENV['AP_SSID_PREFIX']        ||= 'Spark WiFi'
ENV['AP_IP']                 ||= '192.168.1.1/24'
ENV['LOG_FILE']              ||= '/var/log/syslog'

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
end

