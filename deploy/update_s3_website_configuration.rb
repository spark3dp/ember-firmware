#!/usr/bin/env ruby

# Updates AWS S3 static hosting routing rules redirect with new destination
# Sample current configuration:
=begin
{
    "IndexDocument": {
        "Suffix": "index.html"
    },
    "RoutingRules": [
        {
            "Redirect": {
                "ReplaceKeyWith": "printer-firmware/smith-1.0.1203.0.tar",
                "HostName": "s3.amazonaws.com",
                "Protocol": "https",
                "HttpRedirectCode": "302"
            },
            "Condition": {
                "KeyPrefixEquals": "latest_firmware"
            }
        },
        {
            "Redirect": {
                "ReplaceKeyWith": "printer-firmware/smith-1.1.0123.0.tar",
                "HostName": "s3.amazonaws.com",
                "Protocol": "https",
                "HttpRedirectCode": "302"
            },
            "Condition": {
                "KeyPrefixEquals": "testing_firmware"
            }
        }
    ]
}
=end

require 'json'

# The first argument is the current configuration
if !(current_config_json = ARGV[0])
  abort('Current configuration JSON must be specified as first argument')
end

# The second argument is the KeyPrefixEquals condition of the RoutingRule to update
if !(redirect_condition = ARGV[1])
  abort('KeyPrefixEquals condition must be specified as second argument')
end

# The third value is the new destination
if !(new_destination = ARGV[2])
  abort('New destination must be specified as third argument')
end

config = JSON.parse(current_config_json)

index_of_rule_to_update = config['RoutingRules'].map { |rr| rr['Condition']['KeyPrefixEquals'] }.index(redirect_condition)

config['RoutingRules'][index_of_rule_to_update]['Redirect']['ReplaceKeyWith'] = "printer-firmware/#{new_destination}"

STDOUT.print(config.to_json)
