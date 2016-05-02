#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
rsync \
-auv \
--delete \
--exclude=deploy/setup/main/firmware \
--exclude=deploy/deploy \
--exclude=deploy/.git* \
--exclude=deploy/git \
--exclude=oib.log \
--exclude=.*.swp \
--exclude=*.DS_Store \
--exclude=deploy/ignore \
--exclude=.project \
--exclude=deploy/omap-image-builder \
"${SCRIPTPATH}" root@192.168.7.2:~/
