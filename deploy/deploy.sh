#!/bin/bash

SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )
rsync -auv --delete --exclude=deploy/deploy --exclude=deploy/git --exclude=.*.swp "${SCRIPTPATH}" bbb:~/
