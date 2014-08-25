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
chroot "${FS_ROOT}" gem list | cut -d" " -f1 | chroot "${FS_ROOT}" xargs gem uninstall -aIx --bindir /usr/local/bin  > /dev/null 2>&1 || true
# Get gem installation path
gem_dir="${FS_ROOT}$(chroot "${FS_ROOT}" gem environment | grep -Po 'INSTALLATION DIRECTORY: \K[^"]*')"
# Install smith gem
(cd /root/cache && GEM_HOME="${gem_dir}" GEM_PATH="${gem_dir}" gem install smith --local --no-ri --no-rdoc --install-dir "${gem_dir}" --bindir "${FS_ROOT}/usr/local/bin")
