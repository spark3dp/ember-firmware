#!/bin/bash -e

setup_dir=$(cd $(dirname "$0"); pwd)

Red='\e[0;31m'
Gre='\e[0;32m'
Yel='\e[0;33m'
RCol='\e[0m'

temperature_file=/mnt/1wire/22.*/fasttemp
usb_hub_name='Cypress Semiconductor Corp'

test_temperature_sensor() {
  echo -e "${Gre}Testing 1-wire temperature sensor${RCol}"
  if ls ${temperature_file} > /dev/null 2>&1; then
    echo -e "${Gre}PASSED: Temperature sensor found, current reading: $(cat ${temperature_file})${RCol}"
  else
    echo -e "${Red}FAILED: Temperature sensor not found${RCol}"
    exit 1
  fi
}

test_usb_hub() {
  echo -e "${Gre}Testing USB hub${RCol}"
  # Check for presence of Cypress USB hub
  usb_hub=$(lsusb | grep "${usb_hub_name}") || true
  if [[ ! -z "${usb_hub}" ]]; then
    echo -e "${Gre}PASSED: USB hub found: ${usb_hub}${RCol}"
  else
    echo -e "${Red}FAILED: USB hub not found${RCol}"
    exit 1
  fi
}

echo
echo 'Autodesk Isaac main board bring-up step 2 script'
echo
"${setup_dir}/setup_emmc.sh"
echo
test_temperature_sensor
echo
test_usb_hub
echo
echo -e "${Gre}All tests PASSED${RCol}"
