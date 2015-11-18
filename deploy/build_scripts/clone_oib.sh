#!/bin/bash -e
# Specifying the e flag causes the script to abort and exit with failure code if any command in the script fails

oib_git_url='https://github.com/RobertCNelson/omap-image-builder.git'

# This is the release we know works with our build scripts
oib_release='v2015.11'

if [ ! -d omap-image-builder ]; then
  git clone ${oib_git_url}
fi

cd omap-image-builder && git checkout master && git fetch && git checkout tags/${oib_release}
