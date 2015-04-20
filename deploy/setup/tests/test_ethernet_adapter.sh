#!/bin/bash -e

Red='\e[0;31m'
Gre='\e[0;32m'
RCol='\e[0m'

adapter=eth0
host='192.168.10.1'

echo "Testing ethernet adapter ${adapter}"
if ip link show "${adapter}" > /dev/null 2>&1; then

  # Test for connectivity

  if ping -c 1 -w 5 $host > /dev/null 2>&1; then
    echo -e "${Gre}PASSED: Network adapter ${adapter} found and network is reachable${RCol}"
  else
    echo -e "${Red}FAILED: Network adapter ${adapter} found but network is not reachable${RCol}"
    exit 1
  fi

else
  echo -e "${Red}FAILED: Network adapter ${adapter} not found${RCol}"
  exit 1
fi
