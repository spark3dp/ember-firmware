#!/bin/bash -e

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

usb_hub_name='Genesys Logic, Inc. USB-2.0 4-Port HUB'
expected_count=2

echo 'Testing for presence of external USB hubs'
count=$(lsusb | grep "${usb_hub_name}" | wc -l) || true
if [[ "${count}" -eq "${expected_count}" ]]; then
  echo -e "${Gre}PASSED: External USB hubs properly enumerated${RCol}"
else
  echo -e "${Red}FAILED: External USB hubs not properly enumerated (expected to find ${expected_count} but found ${count})${RCol}"
  exit 1
fi
