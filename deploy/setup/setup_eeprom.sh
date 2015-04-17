#!/bin/bash -e

# This is written after the serial number to indicate the hardware revision/configuration
hw_rev=000000

# Number of bytes to skip before writing hardware revision
hw_rev_offset=32

# This is prepended to the serial number
serial_number_prefix=0

# Length of expected serial number
expected_serial_number_length=11

# Number of bytes to skip before writing serial number
serial_number_offset=16

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

echo -n 'Enter the board serial number followed by enter key (or new line): '
read  serial_number

if [[ -z "${serial_number}" ]]; then
  echo -e "${Red}ERROR: Serial number must not be empty, exiting${RCol}"
  exit 1
fi

if [[ "${#serial_number}" -ne "${expected_serial_number_length}" ]]; then
  echo -e "${Red}ERROR: Serial number must be ${expected_serial_number_length} characters, exiting${RCol}"
  exit 1
fi

# Quiet option to suppress output can be specified as second argument
if [[ "${1}" == '--quiet' ]]; then
  exec &> /dev/null
fi

eeprom="/sys/bus/i2c/devices/0-0050/eeprom"
eeprom_location=$(ls /sys/devices/ocp.*/44e0b000.i2c/i2c-0/0-0050/eeprom 2> /dev/null)
setup_dir=$(cd $(dirname "$0"); pwd)

# Write the header so the kernel can properly load the device tree
echo -e "${Gre}Writing header to EEPROM${RCol}"
dd if="${setup_dir}/eeprom/bbb-eeprom.dump" of="${eeprom_location}"

# Check if header was written correctly
if [[ "x$(hexdump -e '8/1 "%c"' ${eeprom} -s 5 -n 3)" == "x335" ]]; then
  header_valid=true
else
  header_valid=false
fi

# Write serial number
serial_number="${serial_number_prefix}${serial_number}"
serial_number_length=${#serial_number}

echo -e "${Gre}Writing serial number ${serial_number} to EEPROM${RCol}"
echo -n "${serial_number}" | dd obs=1 seek=$serial_number_offset of="${eeprom_location}"

# Check if serial number was written correctly
if [[ "x$(hexdump -e '8/1 "%c"' ${eeprom} -s $serial_number_offset -n $serial_number_length)" == "x${serial_number}" ]]; then
  serial_number_valid=true
else
  serial_number_valid=false
fi

# Write the hardware revision
hw_rev_length=${#hw_rev}

echo -e "${Gre}Writing hardware revision ${hw_rev} to EEPROM${RCol}"
echo -n "${hw_rev}" | dd obs=1 seek=$hw_rev_offset of="${eeprom_location}"

# Check if serial number was written correctly
if [[ "x$(hexdump -e '8/1 "%c"' ${eeprom} -s $hw_rev_offset -n $hw_rev_length)" == "x${hw_rev}" ]]; then
  hw_rev_valid=true
else
  hw_rev_valid=false
fi

if [[ $header_valid == true ]] && [[ $serial_number_valid == true ]] && [[ $hw_rev_valid == true ]]; then
  echo -e "${Gre}Done, reboot required${RCol}"
else
  echo -e "${Red}EEPROM was not written successfully. Verify that WP is pulled low to enable writing to the EEPROM.${RCol}"
  exit 1
fi
