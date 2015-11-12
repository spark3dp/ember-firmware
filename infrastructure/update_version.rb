#!/usr/bin/env ruby

# Updates BUILD_DATE and BUILD_NUMBER definition in specified file
# Writes version.properties file for injection of firmware version into Bamboo

DATE_REGEX = /constexpr\s+const\s+char\s*\*\s*BUILD_DATE\s*=\s*"(.+?)"\s*;/
NUMBER_REGEX = /constexpr\s+const\s+char\s*\*\s*BUILD_NUMBER\s*=\s*"(.+?)"\s*;/
MAJOR_REGEX = /constexpr\s+const\s+char\s*\*\s*VERSION_MAJOR\s*=\s*"(.+?)"\s*;/
MINOR_REGEX = /constexpr\s+const\s+char\s*\*\s*VERSION_MINOR\s*=\s*"(.+?)"\s*;/

if !(build_h = ARGV[0])
  abort('Path to Build.h must be specified as first argument')
end

if !(shared_h= ARGV[1])
  abort('Path to Shared.h must be specified as second argument')
end

build_h_contents = File.read(build_h)
shared_h_contents = File.read(shared_h)

date_match_groups = build_h_contents.match(DATE_REGEX)
number_match_groups = build_h_contents.match(NUMBER_REGEX)
major_match_groups = shared_h_contents.match(MAJOR_REGEX)
minor_match_groups = shared_h_contents.match(MINOR_REGEX)

if !date_match_groups || !number_match_groups || !major_match_groups || !minor_match_groups
  abort('Unable to match existing preprocessor definitions in specified header files')
end

current_build_date = date_match_groups[1]
current_build_number = number_match_groups[1]
version_major = major_match_groups[1]
version_minor = minor_match_groups[1]

build_date = Time.now.strftime('%Y%m%d')

if build_date == current_build_date
  build_number = ((current_build_number.to_i) + 1).to_s
else
  build_number = '0'
end

new_contents = build_h_contents.
  sub(DATE_REGEX, "constexpr const char* BUILD_DATE = \"#{build_date}\";").
  sub(NUMBER_REGEX, "constexpr const char* BUILD_NUMBER = \"#{build_number}\";")

new_version ="#{version_major}.#{version_minor}.#{build_date}.#{build_number}"

File.write(build_h, new_contents)
File.write('version.properties', "firmwareVersion=#{new_version}")

STDOUT.print(new_version)
