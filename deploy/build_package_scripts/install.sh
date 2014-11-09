#!/bin/bash -e
# Specifying the e flag causes the script to abort and exit with failure code if any command in the script fails

# The root of the firmware image filesystem
FS_ROOT="$1"

# Restore the gemrc file after renaming
restore_gemrc() {
  if [ -f /etc/gemrc.tmp ]; then
    mv -f /etc/gemrc.tmp /etc/gemrc
  fi
}

# Copy the smith executable

echo 'Installing smith executable'
rm -f "${FS_ROOT}/usr/local/bin/smith"
cp -v /smith/smith "${FS_ROOT}/usr/local/bin"

echo 'Installing smith gem'

# Uninstall any existing gems
chroot "${FS_ROOT}" gem list | cut -d" " -f1 | chroot "${FS_ROOT}" xargs gem uninstall -aIx --bindir /usr/local/bin  > /dev/null 2>&1 || true

# Get gem installation path
gem_home="${FS_ROOT}$(chroot "${FS_ROOT}" gem environment | grep -Po 'INSTALLATION DIRECTORY: \K[^"]*')"

# It is necessary to temporarily disable the settings from /etc/gemrc since apparently providing
# GEM_HOME and GEM_PATH variables on the command line with the gem command does not override the
# settings in the gemrc file.  If GEM_HOME and GEM_PATH are not overridden, then all the gems will
# fail to install into the new filesystem.  The file is restored in an exit hook.
trap restore_gemrc EXIT
mv -f /etc/gemrc /etc/gemrc.tmp

# Install smith gem
# All this path adjustment trickery is used so we can install the gems within the context of the
# build system to have access to the compilation toolchain for building native extensions.
# It would be cleaner to run "gem install" within a chroot jail but then the compilation tools
# would have to be installed on the release filesystem but there is no need for those tools
# to be present on the release filesystem other than to compile gem native extensions when
# the gems are installed
(cd /root/cache && GEM_HOME="${gem_home}" GEM_PATH="${gem_home}" gem install smith --local --no-ri --no-rdoc --install-dir "${gem_home}" --bindir "${FS_ROOT}/usr/local/bin")
