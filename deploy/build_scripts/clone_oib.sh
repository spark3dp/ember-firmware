#!/bin/bash -e
# Specifying the e flag causes the script to abort and exit with failure code if any command in the script fails

oib_git_url='https://github.com/RobertCNelson/omap-image-builder.git'

# This is the commit we know works with our build scripts
# omap-image-builder release v2015.10
oib_commit_sha1='1ed9af7ce686250c5fa20fed18c097d3569df65c'

if [ ! -d omap-image-builder ]; then
  git clone ${oib_git_url}
fi

cd omap-image-builder && git checkout master && git pull origin master && git checkout ${oib_commit_sha1}
