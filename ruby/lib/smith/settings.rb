# Settings is an instance of Struct with methods corresponding to setting names.
# Setting values can be specified through the environment or through default values.
# The default value is used if the environment variable for a given setting is not set.
# Settings can be accessed and modified at runtime by calling the method corresponding
# to the setting name on the Settings object. If a value is updated with a setter method
# the change is not persisted and will last only until execution completes. 
# Environment overrides are only applied when this file is required (at startup)
#
# Example usage:
#
# sample_setting: ENV['SAMPLE'] || 'sample_value'
#
# Settings.sample_setting
# => "sample_value"
#
# Settings.sample_setting = 'other_value'
# Settings.sample_setting
# => "other_value"

module Smith
  settings = {
    # setting name          environment key                        default value
    
    # wpa_supplicant wireless network configuration file path
    wpa_roam_file:          ENV['SMITH_WPA_ROAM_FILE']          || '/var/local/wpa-roam.conf',

    # hostapd configuration file path
    hostapd_config_file:    ENV['SMITH_HOSTAPD_CONFIG_FILE']    || '/var/local/hostapd.conf',
    
    # dnsmasq configuration file path
    dnsmasq_config_file:    ENV['SMITH_DNSMASQ_CONFIG_FILE']    || '/var/local/dnsmasq.conf',

    # Program state backing file
    state_file:             ENV['SMITH_STATE_FILE']             || '/var/local/smith_state',

    # Path of named pipe used to issue commands to smith
    command_pipe:           ENV['SMITH_COMMAND_PIPE']           || COMMAND_PIPE,

    # Path of named pipe used to read response values from issued commands
    command_response_pipe:  ENV['SMITH_COMMAND_RESPONSE_PIPE']  || COMMAND_RESPONSE_PIPE,

    # Print file download/upload directory
    print_data_dir:         ENV['SMITH_PRINT_DATA_DIR']         || '/var/smith/download',

    # Firmware images directory
    firmware_dir:           ENV['SMITH_FIRMWARE_DIR']           || '/main/firmware',

    # Firmware versions file path
    firmware_versions_file: ENV['SMITH_FIRMWARE_VERSIONS_FILE'] || '/main/firmware/versions',

    # Wireless interface path
    wireless_interface:     ENV['SMITH_WIRELESS_INTERFACE']     || 'wlan0',

    # Wired interface path
    wired_interface:        ENV['SMITH_WIRED_INTERFACE']        || 'eth0',

    # Access point mode SSID prefix
    ap_ssid_prefix:         ENV['SMITH_AP_SSID_PREFIX']         || 'Spark WiFi',

    # Wireless interface IP address including subnet mask for access point mode
    ap_ip_address:          ENV['SMITH_AP_IP_ADDRESS']          || '192.168.1.1/24',

    # syslog directory
    log_dir:                ENV['SMITH_LOG_DIR']                || '/var/log',

    # Spark backend server URL
    server_url:             ENV['SMITH_SERVER_URL']             || 'http://printer-backend-dev.ngrok.com',

    # Spark backend server API version for generating URL
    server_api_version:     ENV['SMITH_SERVER_API_VERSION']     || 'v1',

    # HTTP endpoints used by client to make requests to server
    client_endpoint:        ENV['SMITH_CLIENT_ENDPOINT']        || 'faye',
    registration_endpoint:  ENV['SMITH_REGISTRATION_ENDPOINT']  || 'printers',
    acknowledge_endpoint:   ENV['SMITH_ACKNOWLEDGE_ENDPOINT']   || 'printers/<%= printer_id %>/acknowledge',

    # Faye channels used by client to receive notifications from server
    registration_channel:   ENV['SMITH_REGISTRATION_CHANNEL']   || '/printers/<%= printer_id %>/users',
    command_channel:        ENV['SMITH_COMMAND_CHANNEL']        || '/printers/<%= printer_id %>/command',

    # File that registration values intended to be displayed on front panel are written to
    registration_info_file: ENV['SMITH_REGISTRATION_INFO_FILE'] || PRIMARY_REGISTRATION_INFO_FILE,

    # File that settings are written to for communication to smith 
    print_settings_file:    ENV['SMITH_PRINT_SETTINGS_FILE']    || PRINT_SETTINGS_FILE,

    # AWS S3 bucket name that log file archives are uploaded to
    s3_log_bucket:          ENV['SMITH_S3_LOG_BUCKET']          || 'ember-log-archives',

    # AWS access key for ember_printer user
    aws_access_key_id:      ENV['SMITH_AWS_ACCESS_KEY_ID']      || 'AKIAIEEHFUR53SNDSFMA',

    # AWS secret access key for ember_printer user
    aws_secret_access_key:  ENV['SMITH_AWS_SECRET_ACCESS_KEY']  || 'DvyiDCGA6HTkJYItZyRmWX4pHv6Ck0S80hQtX5Z1',

    # AWS region for S3 bucket
    aws_region:             ENV['SMITH_AWS_REGION']             || 'us-east-1'
  }

  Settings = Struct.new(*settings.keys).new(*settings.values)
end
