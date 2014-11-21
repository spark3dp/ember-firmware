#!/bin/bash -e

setup_dir=$(cd $(dirname "$0"); pwd)

Red='\e[0;31m'
Gre='\e[0;32m'
Yel='\e[0;33m'
RCol='\e[0m'

echo
echo 'Autodesk Isaac main board bring-up step 1 script'
echo
echo -n -e "${Yel}Enter the board serial number followed by enter key (or new line): ${RCol}"
read  serial_number

if [[ -z "${serial_number}" ]]; then
  echo -e "${Red}Serial number must not be empty, exiting${RCol}"
  exit 1
fi

echo
"${setup_dir}/setup_eeprom.sh" "${serial_number}"
echo
echo -e "${Gre}Rebooting NOW${RCol}"
reboot
