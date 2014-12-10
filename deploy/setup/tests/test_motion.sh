#!/bin/bash -e
read -p "Press any key to move down 100mm." -n1 -s
echo
zed D100000
read -p "Press any key to return to the top." -n1 -s
echo
zed h
read -p "Press any key for full rotation." -n1 -s
echo
zed R360000
