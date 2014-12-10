#!/usr/bin/env ruby

require 'json'

if ARGV.length < 1
  abort('Must specify valid JSON string as argument')
end

if suffix = JSON.parse(ARGV[0])['ssid_suffix']
  print(suffix)
else
  exit 1
end
