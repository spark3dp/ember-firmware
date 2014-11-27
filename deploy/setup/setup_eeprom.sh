#!/bin/bash -e

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

echo -n 'Enter the board serial number followed by enter key (or new line): '
read  serial_number

if [[ -z "${serial_number}" ]]; then
  printf "${Red}ERROR: Serial number must not be empty, exiting${RCol}\n"
  exit 1
fi

# Quiet option to suppress output can be specified as second argument
if [[ "${1}" == '--quiet' ]]; then
  exec &> /dev/null
fi

serial_number_prefix=0042
eeprom="/sys/bus/i2c/devices/0-0050/eeprom"
eeprom_location=$(ls /sys/devices/ocp.*/44e0b000.i2c/i2c-0/0-0050/eeprom 2> /dev/null)
setup_dir=$(cd $(dirname "$0"); pwd)

echo -e "${Gre}Writing header to EEPROM${RCol}"
dd if="${setup_dir}/eeprom/bbb-eeprom.dump" of="${eeprom_location}"

# Check if header was written correctly
if [[ "x$(hexdump -e '8/1 "%c"' ${eeprom} -s 5 -n 3)" == "x335" ]]; then
  header_valid=true
else
  header_valid=false
fi

# Write a serial number if specified
if [[ ! -z "${serial_number}" ]]; then
  serial_number="${serial_number_prefix}${serial_number}"
  serial_number_length=${#serial_number}

  echo -e "${Gre}Writing serial number ${serial_number} to EEPROM${RCol}"
  echo -n "${serial_number}" | dd obs=1 seek=16 of="${eeprom_location}"

  # Check if serial number was written correctly
  if [[ "x$(hexdump -e '8/1 "%c"' ${eeprom} -s 16 -n $serial_number_length)" == "x${serial_number}" ]]; then
    serial_number_valid=true
  else
    serial_number_valid=false
  fi

else
  serial_number_valid=true
fi

if [[ $header_valid == true ]] && [[ $serial_number_valid == true ]]; then
  echo -e "${Gre}Done, reboot required${RCol}"
else
  echo -e "${Red}EEPROM was not written successfully. Verify that WP is pulled low to enable writing to the EEPROM.${RCol}"
  exit 1
fi
