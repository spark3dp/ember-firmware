# Settings is an instance of Struct with methods corresponding to setting names.
# Setting values can be specified through the environment with a key matching "SMITH_"
# concatenated with the setting name in all capital letter or through default values.
# The default value is used if the environment variable for a given setting is not set.
# Settings can be accessed and modified at runtime by calling the method corresponding
# to the setting name on the Settings object. If a value is updated with a setter method
# the change is not persisted and will last only until execution completes. 
# Environment overrides are only applied when this file is required (at startup)
#
# Example usage:
#
# sample_setting: 'sample_value'
#
# Settings.sample_setting
# => "sample_value"
#
# Settings.sample_setting = 'other_value'
# Settings.sample_setting
# => "other_value"
#
# ENV['SMITH_SAMPLE_SETTING'] = 'override'
# sample_setting: 'sample_value'
#
# Settings.sample_setting
# => "override"

require 'smith/settings_factory'

module Smith
  Settings = SettingsFactory.build(
    # setting_name: default_value
    
    # wpa_supplicant wireless network configuration file path
    wpa_roam_file: '/var/local/wpa-roam.conf',

    # hostapd configuration file path
    hostapd_config_file: '/var/local/hostapd.conf',
    
    # dnsmasq configuration file path
    dnsmasq_config_file: '/var/local/dnsmasq.conf',

    # Program state backing file
    state_file: '/var/local/smith_state',

    # Path of named pipe used to issue commands to smith
    command_pipe: COMMAND_PIPE,

    # Path of named pipe used to receive status updates from smith
    status_pipe: STATUS_TO_WEB_PIPE,

    # Print file download/upload directory
    print_data_dir: '/var/smith/download',

    # Firmware images directory
    firmware_dir: '/main/firmware',

    # Firmware versions file path
    firmware_versions_file: '/main/firmware/versions',

    # Wireless interface path
    wireless_interface: 'wlan0',

    # Wired interface path
    wired_interface: 'eth0',

    # Access point mode SSID prefix
    ap_ssid_prefix: 'Ember WiFi',

    # Wireless interface IP address including subnet mask for access point mode
    ap_ip_address: '192.168.1.1/24',

    # syslog directory
    log_dir: '/var/log',

    # Spark backend server URL
    server_url: 'http://api.emberprinter.com',

    # Spark backend server API version for generating URL
    server_api_version: 'v1',

    # HTTP endpoints used by client to make requests to server
    client_endpoint:       'faye',
    registration_endpoint: 'printers',
    acknowledge_endpoint:  'printers/<%= printer_id %>/acknowledge',
    status_endpoint:       'printers/<%= printer_id %>/status',
    health_check_endpoint: 'printers/<%= printer_id %>/health_check',

    # Faye channels used by client to receive notifications from server
    registration_channel: '/printers/<%= printer_id %>/users',
    command_channel:      '/printers/<%= printer_id %>/command',

    # File that registration values intended to be displayed on front panel are written to
    registration_info_file: PRIMARY_REGISTRATION_INFO_FILE,

    # File that settings are written to for communication to smith 
    print_settings_file: TEMP_PRINT_SETTINGS_FILE,

    # AWS S3 bucket name that log file archives are uploaded to
    s3_log_bucket: 'ember-log-archives',

    # AWS access key for ember_printer user
    aws_access_key_id: 'AKIAIEEHFUR53SNDSFMA',

    # AWS secret access key for ember_printer user
    aws_secret_access_key: 'DvyiDCGA6HTkJYItZyRmWX4pHv6Ck0S80hQtX5Z1',

    # AWS region for S3 bucket
    aws_region: 'us-east-1',

    # Interval in seconds between client primary registration attempts
    client_retry_interval: 60,

    # Interval in seconds between client health check requests
    client_health_check_interval: 15,

    # Connection timeout in seconds for POST requests made to server
    post_request_connect_timeout: 10,
    
    # Inactivity timeout in seconds for POST requests made to server
    post_request_inactivity_timeout: 20,

    # Connection timeout in seconds for file download GET requests
    file_download_connect_timeout: 15,
    
    # Inactivity timeout in seconds for file download GET requests
    file_download_inactivity_timeout: 30,

    # smith settings file path
    smith_settings_file: '/var/smith/config/settings',

    # Timeout in seconds for communication with smith
    printer_communication_timeout: 1,

    # Printer status file
    printer_status_file: PRINTER_STATUS_FILE,

    # Time to wait in seconds after processing wireless connection web request but before
    # initiating the actual connection, allows response to be returned to client before adapter
    # leaves access point mode
    wireless_connection_delay: 5,

    # Time to wait in seconds for deciding that a wireless network connection attempt has failed
    wireless_connection_timeout: 10,

    # Time to wait in seconds between checking if wireless connection has been established during polling
    wireless_connection_poll_interval: 0.5
  )

end
