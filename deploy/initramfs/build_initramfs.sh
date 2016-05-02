#!/bin/bash -e

target_dir="${1}"
target=initrd.img-$(uname -r)
initramfs_tools_dir=/usr/share/initramfs-tools
SCRIPTPATH=$( cd $(dirname $0) ; pwd -P )

if ! which mkinitramfs > /dev/null; then
  echo 'initramfs-tools required, install with "apt-get install initramfs-tools", exiting'
  exit 1
fi

if ! which busybox > /dev/null; then
  echo 'busybox required, install with "apt-get install busybox", exiting'
  exit 1
fi

if [ ! -d "${target_dir}" ]; then
  echo "Must specify destination directory of ${target}"
  exit 1
fi

mount -B "${SCRIPTPATH}/initramfs-tools" "${initramfs_tools_dir}"

echo "Building ${target} with kernel version $(uname -r)"

mkinitramfs -d "${SCRIPTPATH}/conf" -o "${SCRIPTPATH}/${target}"

echo "Updating ${target} in ${target_dir}"

if cp -v "${SCRIPTPATH}/${target}" "${target_dir}"; then
  rm -f "${SCRIPTPATH}/${target}"
else
  echo "Could not copy ${target} to ${target_dir}, new image is located at ${SCRIPTPATH}/${target}"
fi

umount "${initramfs_tools_dir}"

echo "Done"
