#!/bin/bash -e

eeprom="/sys/bus/i2c/devices/0-0050/eeprom"
eeprom_location=$(ls /sys/devices/ocp.*/44e0b000.i2c/i2c-0/0-0050/eeprom 2> /dev/null)
setup_dir=$(cd $(dirname "$0"); pwd)

# Serial number can be specified as first argument
serial_number="${1}"

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

check_header() {
  unset header_valid
  if [[ "x$(hexdump -e '8/1 "%c"' ${eeprom} -s 5 -n 3)" == "x335" ]]; then
    header_valid=true
  else
    header_valid=false
  fi
}

check_header
if [[ "${header_valid}" == true ]]; then
  echo -e "${Gre}Valid EEPROM header found. Nothing to do.${RCol}"
  exit 2
else
  echo -e "${Gre}Invalid EEPROM header detected${RCol}"
  echo -e "${Gre}Writing header to EEPROM${RCol}"
  dd if="${setup_dir}/eeprom/bbb-eeprom.dump" of="${eeprom_location}"
  check_header

  # Write a serial number if specified
  if [[ ! -z "${serial_number}" ]]; then
    echo -e "${Gre}Writing serial number ${serial_number} to EEPROM${RCol}"
    echo -n "${serial_number}" | dd obs=1 seek=16 of="${eeprom_location}"
  fi

  if [[ "${header_valid}" == true ]]; then
    echo -e "${Gre}Done, reboot required${RCol}"
  else
    echo -e "${Red}EEPROM header is invalid after writing. Verify that WP is pulled low to enable writing to the EEPROM.${RCol}"
    exit 1
  fi
fi
