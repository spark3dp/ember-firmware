#!/bin/bash -e
# Specifiying the e flag causes the script to abort and exit with failure code if any command in the script fails

# The root of the firmware image filesystem
FS_ROOT="$1"

# Copy the smith executable
echo 'Installing smith executable'
rm -f "${FS_ROOT}/usr/local/bin/smith"
cp -v /smith/smith "${FS_ROOT}/usr/local/bin"

echo 'Installing smith gem'
# Uninstall any existing gems
chroot "${FS_ROOT}" gem list | cut -d" " -f1 | chroot "${FS_ROOT}" xargs gem uninstall -aIx > /dev/null 2>&1 || true 
# Get gem installation directory
gem_home="${FS_ROOT}$(chroot "${FS_ROOT}" gem environment | grep -Po 'INSTALLATION DIRECTORY: \K[^"]*')"
# Install smith gem
(cd /root/cache && GEM_HOME="${gem_home}" GEM_PATH="${gem_home}" gem install smith --local --no-ri --no-rdoc --bindir "${FS_ROOT}/usr/local/bin")
