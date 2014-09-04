#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
rsync -auv --delete --exclude=deploy/setup/main/firmware --exclude=deploy/deploy --exclude=deploy/git --exclude=oib.log --exclude=.*.swp "${SCRIPTPATH}" bbb:~/
