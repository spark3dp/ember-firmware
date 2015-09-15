#  File: settings.rb
#  Specifies settings and constructs settings instance
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
    state_file: SMITH_STATE_FILE,

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
    server_api_namespace: 'api/v1/print',

    # HTTP endpoints used by client to make requests to server
    client_endpoint:       'faye',
    registration_endpoint: 'printers/registration_code',
    acknowledge_endpoint:  'printers/command/<%= task_id %>',
    status_endpoint:       'printers/status',

    # Faye channels used by client to receive notifications from server
    registration_channel: '/printers/<%= printer_id %>/users',
    command_channel:      '/printers/<%= printer_id %>/command',

    # File that registration values intended to be displayed on front panel are written to
    registration_info_file: PRIMARY_REGISTRATION_INFO_FILE,

    # File that settings are written to for communication to smith 
    settings_file: TEMP_SETTINGS_FILE,

    # Interval in seconds between client primary registration attempts
    client_retry_interval: 60,

    # Interval in seconds between client periodic status requests
    client_periodic_status_interval: 15,

    # Connection timeout in seconds for POST requests made to server
    post_request_connect_timeout: 10,
    
    # Inactivity timeout in seconds for POST requests made to server
    post_request_inactivity_timeout: 20,

    # Connection timeout in seconds for file download GET requests
    file_download_connect_timeout: 15,
    
    # Inactivity timeout in seconds for file download GET requests
    file_download_inactivity_timeout: 30,

    # Connection timeout in seconds for file upload requests
    file_upload_connect_timeout: 15,

    # Inactivity timeout in seconds for file upload requests
    file_upload_inactivity_timeout: 30,

    # smith settings file path
    smith_settings_file: ROOT_DIR+SETTINGS_SUB_DIR+SETTINGS_FILE,

    # Timeout in seconds for communication with smith
    printer_communication_timeout: 1,

    # Printer status file
    printer_status_file: PRINTER_STATUS_FILE,

    # Time to wait in seconds after processing wireless connection web request but before
    # initiating the actual connection, allows response to be returned to client before adapter
    # leaves access point mode
    wireless_connection_delay: 5,

    # Time to wait in seconds for deciding that a wireless network connection attempt has failed
    wireless_connection_timeout: 60,

    # Time to wait in seconds between checking if wireless connection has been established during polling
    wireless_connection_poll_interval: 0.5,

    # The ID Spark uses for Ember printers
    spark_printer_type_id: '7FAF097F-DB2E-45DC-9395-A30210E789AA'

  )

end
