#!/bin/bash -e
# Specifying the e flag causes the script to abort and exit with failure code if any command in the script fails

oib_git_url='https://github.com/RobertCNelson/omap-image-builder.git'

# This is the commit we know works with our build scripts
oib_commit_sha1='72b98d58a3e9f88561b8346c4752510a56a2a173'

if [ ! -d omap-image-builder ]; then
  git clone ${oib_git_url}
fi

cd omap-image-builder && git checkout master && git pull origin master && git checkout ${oib_commit_sha1}
