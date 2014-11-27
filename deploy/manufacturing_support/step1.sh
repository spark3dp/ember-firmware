#!/usr/bin/env bash

user=root
host='192.168.7.2'
timeout=120 # seconds
setup_dir='/root/deploy/setup'

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

# Wait until communication with a host on the network is possible
wait_for_connectivity() {
  echo 'Waiting for board to be ready...'
  echo
  # Ping at an interval of 0.1 seconds until some timeout expires
  if ! ping -o -i 0.1 -t $timeout "${host}" > /dev/null; then
    printf "${Red}ERROR: Unable to reach main board, make sure USB is connected and try again, aborting${RCol}\n"
    exit 1
  fi
}

# Issue specified command(s) over ssh
send_command() {
  # Verify that a command is specified
  command="${@}"
  if [[ -z "${command}" ]]; then
    echo 'Command must be specified'
    exit 1
  fi
  
  ssh "${user}@${host}" "${command}"
}

echo
echo 'Autodesk Isaac main board bring-up script'
echo

wait_for_connectivity
echo 'Running step 1...'
echo
send_command "${setup_dir}/setup_eeprom.sh" --quiet

if [[ $? -eq 0 ]]; then
  printf "\n${Gre}Step 1 SUCCESSFUL, press any key to reboot the board${RCol}\n"
  read -n 1
  printf 'Rebooting...\n\n'
  #send_command reboot
else
  printf "${Red}Step 1 FAILED, aborting${RCol}\n"
  exit 1
fi

#sleep 10
wait_for_connectivity
echo 'Running step 2...'

# Retry the emmc setup script if it fails since the partition table may be busy after formatting
n=0
emmc_setup_complete=false
until [ $n -ge 5 ] # Retry up to 5 times
do
  if send_command "${setup_dir}/setup_emmc.sh" --quiet; then
    emmc_setup_complete=true
    break
  fi
  n=$[$n+1]
done

if [[ "${emmc_setup_complete}" == true ]]; then
  printf "\n${Gre}Step 2 SUCCESSFUL${RCol}\n"
else
  printf "${Red}Step 2 FAILED, aborting${RCol}\n"
  exit 1
fi

echo 'Running tests...'
echo

tests_passed=true

if ! send_command "${setup_dir}/tests/test_temperature_sensor.sh"; then
  tests_passed=false
fi
echo
if ! send_command "${setup_dir}/tests/test_usb_hub.sh"; then
  tests_passed=false
fi
echo
if ! send_command "${setup_dir}/tests/test_ethernet_adapter.sh"; then
  tests_passed=false
fi
echo
if ! send_command "${setup_dir}/tests/test_wireless_adapter.sh"; then
  tests_passed=false
fi

if [[ $tests_passed == true ]]; then
  printf "\n${Gre}All tests PASSED${RCol}\n"
else
  printf "${Red}One or more tests FAILED, aborting${RCol}\n"
  exit 1
fi
