#!/bin/bash -e
# Specifying the e flag causes the script to abort and exit with failure code if any command in the script fails

# The root of the firmware image filesystem is the first argument
# Must be an absolute path
# It is required
fs_root="$1"

# The second, third, and fourth arguments are required to be provided together or not at all
# The second argument specifies the path to the smith binary that will be installed to the specified filesystem
# The third argument specifies the path to the directory containing the smith Ruby gem and dependencies to install to the specified filesystem
# The fourth argument specifies the path to the zee executable
if [ -n "${2}" -a -n "${3}" -a -n "${4}" ]; then
  smith_bin="${2}"
  gem_cache_dir="${3}"
  zee_bin="${4}"
else
  smith_bin='/usr/local/bin/smith'
  gem_cache_dir='/root/cache'
  zee_bin='/usr/local/bin/zee'
fi

# Check arguments
if [ "${fs_root:0:1}" != "/" ]; then
  echo "Specified filesystem root ('${fs_root}') is not an absolute path, aborting"
  exit 1
fi

if [ ! -d "${fs_root}" ]; then
  echo "Specified filesystem root directory ('${fs_root}') does not exist, aborting"
  exit 1
fi

if [ ! -d "${gem_cache_dir}" ]; then
  echo "Specified Ruby gem cache directory ('${gem_cache_dir}') does not exist, aborting"
  exit 1
fi

if [ ! -f "${smith_bin}" ]; then
  echo "Specified smith binary ('${smith_bin}') does not exist, aborting"
  exit 1
fi

if [ ! -f "${zee_bin}" ]; then
  echo "Specified zee binary ('${zee_bin}') does not exist, aborting"
  exit 1
fi

# Restore the gemrc file after renaming
restore_gemrc() {
  if [ -f /etc/gemrc.tmp ]; then
    mv -f /etc/gemrc.tmp /etc/gemrc
  fi
}

# Copy the smith executable
echo 'Installing smith executable'
rm -f "${fs_root}/usr/local/bin/smith"
cp -v "${smith_bin}" "${fs_root}/usr/local/bin"

# Copy the zee executable
echo 'Installing zee executable'
rm -f "${fs_root}/usr/local/bin/zee"
cp -v "${zee_bin}" "${fs_root}/usr/local/bin"

echo 'Installing smith gem'

# Uninstall any existing gems
chroot "${fs_root}" gem list | cut -d" " -f1 | chroot "${fs_root}" xargs gem uninstall -aIx --bindir /usr/local/bin  > /dev/null 2>&1 || true

# Get gem installation path
gem_home="${fs_root}$(chroot "${fs_root}" gem environment | grep -Po 'INSTALLATION DIRECTORY: \K[^"]*')"

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
(cd "${gem_cache_dir}" && GEM_HOME="${gem_home}" GEM_PATH="${gem_home}" gem install smith --local --no-ri --no-rdoc --install-dir "${gem_home}" --bindir "${fs_root}/usr/local/bin")
