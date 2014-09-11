#!/bin/bash -e

eeprom="/sys/bus/i2c/devices/0-0050/eeprom"
eeprom_location=$(ls /sys/devices/ocp.*/44e0b000.i2c/i2c-0/0-0050/eeprom 2> /dev/null)
eeprom_header=
setup_dir=$(cd $(dirname "$0"); pwd)

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
  exit
else
  echo -e "${Red}Invalid EEPROM header detected${RCol}"
  echo -e "${Gre}Writing to EEPROM${RCol}"
  dd if="${setup_dir}/eeprom/bbb-eeprom.dump" of="${eeprom_location}"
  check_header
  if [[ "${header_valid}" == true ]]; then
    echo -e "${Gre}Done, please reboot${RCol}"
  else
    echo -e "${Red}EEPROM is still invalid after writing. Remember to pull TP4 low to enable writing to the EEPROM.${RCol}"
  fi
fi
