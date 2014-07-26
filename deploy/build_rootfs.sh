#!/bin/bash -e

loopdev=/dev/loop0
fsname=rootfs
mntpt=/mnt

if ! which pv > /dev/null; then
  echo "pv required, install with apt-get install pv, exiting"
  exit 1
fi

if [ ! -f "${1}" ]; then
  echo "Must specify path to filesystem archive, exiting"
  exit 1
fi

if [ -f "${fsname}" ]; then
  echo "Output file (${fsname}) already exists, exiting"
  exit 1
fi

fs_archive="${1}"

size_b=$(stat -c%s "${fs_archive}")
size_kb=$(expr "${size_b}" / 1024)
size=$(expr "${size_kb}" + 30000)
size_mb=$(expr "${size}" / 1024)

echo "Creating filesystem file (~${size_mb}MB)"
fallocate -l "${size}K" "${fsname}"

echo "Attaching filesystem file to ${loopdev}"
losetup "${loopdev}" "${fsname}"

echo "Making filesystem"
mkfs.ext4 -m 1 -v "${loopdev}" >/dev/null 2>&1

echo "Mounting filesystem"
mount -t ext4 "${loopdev}" "${mntpt}"

echo "Extracting filesystem"
pv "${fs_archive}" | tar --numeric-owner --preserve-permissions -xf - -C "${mntpt}" >/dev/null 2>&1

echo "Done extracting, unmounting and detaching filesystem file"
umount "${mntpt}"
losetup -d "${loopdev}"

echo "Done"
