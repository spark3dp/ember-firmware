#!/bin/bash -e

img_name=rootfs.img
tmpfs="${PWD}/tmpfs"
fs_archive="${1}"

if ! which pv > /dev/null; then
  echo "pv required, install with 'apt-get install pv', exiting"
  exit 1
fi

if ! which mksquashfs > /dev/null; then
  echo "squashfs-tools required, install with 'apt-get install squashfs-tools', exiting"
  exit 1
fi

if [ ! -f "${fs_archive}" ]; then
  echo "Must specify path to filesystem archive, exiting"
  exit 1
fi

if [ -f "${image_name}" ]; then
  echo "Output file (${image_name}) already exists, exiting"
  exit 1
fi

echo "Extracting filesystem"
rm -rf "${tmpfs}"
mkdir "${tmpfs}"
pv "${fs_archive}" | tar --numeric-owner --preserve-permissions -xf - -C tmpfs >/dev/null 2>&1

echo "Creating squashfs image"
mksquashfs "${tmpfs}" "${img_name}"

echo "Cleaning up"
rm -rf "${tmpfs}"

echo "Done"
