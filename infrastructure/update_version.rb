#!/usr/bin/env ruby

# Updates FIRMWARE_VERSION definition in specified file
# Writes version.properties file for injection of firmware version into Bamboo

MAJOR_MINOR_REGEX = /#define FIRMWARE_VERSION "(.*?)"/

if !(shared_header = ARGV[0])
  abort('Path to shared header file to update must be specified as first argument')
end

if !(changelist = ARGV[1])
  abort('Changelist must be specified as second argument')
end

header_contents = File.read(shared_header)

if !(match_groups = header_contents.match(MAJOR_MINOR_REGEX))
  abort('Unable to match existing #define in shared header file')
end

new_version = "#{match_groups[1]}.#{changelist}"
new_contents = header_contents.sub(/#define FIRMWARE_VERSION ".*?"/, "#define FIRMWARE_VERSION \"#{new_version}\"")

File.write(shared_header, new_contents)
File.write('version.properties', "firmwareVersion=#{new_version}")

