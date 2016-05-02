#!/bin/bash -e

# Quiet option to suppress output can be specified as second argument
if [[ "${1}" == '--quiet' ]]; then
  exec &> /dev/null
fi

disk=/dev/mmcblk1
setup_dir=$(cd $(dirname "$0"); pwd)

Red='\e[0;31m'
Gre='\e[0;32m'
Yel='\e[0;33m'
RCol='\e[0m'

check_for_disk() {
  if ! file "${disk}" > /dev/null 2>&1; then
    echo -e "${Red}${disk} does not exist. Make sure that the EEPROM has been flashed.\naborting${RCol}"
    exit 1
  fi
}

ensure_unmounted() {
  # Make sure any existing partitions are unmounted
  umount "${disk}p1" > /dev/null 2>&1 || true
  umount "${disk}p2" > /dev/null 2>&1 || true
}

partition() {
sfdisk --in-order --Linux --unit M ${disk} <<-__EOF__
1,50,0xE,*
,,,-
__EOF__
sync
}

format() {
  mkfs.vfat -F 16 "${disk}p1" -n boot
  mkfs.ext4 -F "${disk}p2" -L main
}

mount_partitions() {
  mkdir -p /mnt/boot
  mkdir -p /mnt/main
  mount "${disk}p1" /mnt/boot
  mount "${disk}p2" /mnt/main
}

copy_files() {
  # Copy the bootloader files
  cp -r "${setup_dir}/u-boot/"* /mnt/boot

  # Copy the initial ram disk image, kernel image, uEnv.txt file, and device tree binary
  mkdir /mnt/boot/boot
  cp -r "${setup_dir}/boot/"* /mnt/boot/boot

  # Copy the initial firmware image and versions file
  cp --archive "${setup_dir}/main/firmware" /mnt/main

  # Create a directory for bind-mounting to /tmp
  mkdir /mnt/main/tmp

  # Extract the var skeleton
  tar xf "${setup_dir}/main/var.tar" -C /mnt/main
  sync
}

unmount_partitions() {
  umount /mnt/boot
  umount /mnt/main
  rmdir /mnt/boot
  rmdir /mnt/main
}

check_for_disk
echo 'eMMC setup script'
ensure_unmounted
echo
echo -e "${Gre}Partitioning ${disk}${RCol}"
partition
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Formatting partitions${RCol}"
format
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Mounting partitions${RCol}"
mount_partitions
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Copying files${RCol}"
copy_files
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Unmounting${RCol}"
unmount_partitions
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Setup completed successfully${RCol}"
