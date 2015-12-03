#!/bin/bash -e
# Specifying the e flag causes the script to abort and exit with failure code if any command in the script fails

oib_git_url='https://github.com/RobertCNelson/omap-image-builder.git'

# This is the commit we know works with our build scripts
oib_commit='9644d52551e34d3a4a8d2f6d61e05470a6fd05df'

if [ ! -d omap-image-builder ]; then
  git clone ${oib_git_url}
fi

cd omap-image-builder && git checkout master && git fetch && git checkout ${oib_commit}
