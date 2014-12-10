#!/bin/bash -e

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

temperature_file=/mnt/1wire/22.*/fasttemp

temp_max=40
temp_min=15

echo 'Testing 1-wire temperature sensor'

# Test for presence
if ls ${temperature_file} > /dev/null 2>&1; then
  temp=$(cat ${temperature_file})
  
  printf -v int_temp %.0f "${temp}"

  # Test that the reading is reasonable
  if [[ "${int_temp}" -ge "${temp_min}" ]] && [[ "${int_temp}" -le "${temp_max}" ]]; then
    echo -e "${Gre}PASSED: Temperature sensor found, current reading (${temp}) is within allowable range${RCol}"
  else
    echo -e "${Red}FAILED: Temperature sensor found but current reading (${temp}) is outside of allowable range [${temp_min}-${temp_max}]${RCol}"
    exit 1
  fi

else
  echo -e "${Red}FAILED: Temperature sensor not found${RCol}"
  exit 1
fi
