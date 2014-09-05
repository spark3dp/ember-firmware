#!/bin/bash -e

kernel_ver='3.8.13-bone63'
disk=/dev/mmcblk1
setup_dir=$(cd $(dirname "$0"); pwd)

Red='\e[0;31m'
Gre='\e[0;32m'
Yel='\e[0;33m'
RCol='\e[0m'

confirm() {
  echo -e "${Gre}Summary of block devices:${RCol}"
  lsblk
  echo
  echo -ne "${Yel}The target device is ${disk}. Make sure this is correct as it will be erased. Type in yes to continue: ${RCol}"

  unset confirm
  read confirm
  if [[ "${confirm}" != 'yes' ]]; then
    echo -e "${Red}aborting${RCol}"
    exit
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
  mkfs.ext4 "${disk}p2" -L main
}

mount_partitions() {
  mkdir -p /mnt/boot
  mkdir -p /mnt/main
  mount "${disk}p1" /mnt/boot
  mount "${disk}p2" /mnt/main
}

copy_files() {
  cp -r "${setup_dir}/boot/${kernel_ver}/"* /mnt/boot
  cp -r "${setup_dir}/main/firmware" /mnt/main
  tar xf "${setup_dir}/main/var.tar" -C /mnt/main
  sync
}

unmount_partitions() {
  umount /mnt/boot
  umount /mnt/main
  rmdir /mnt/boot
  rmdir /mnt/main
}

echo 'eMMC setup script'
echo "Using boot files for kernel version ${kernel_ver}"
echo
confirm
ensure_unmounted
echo
echo -e "${Gre}Partitioning ${disk}${RCol}"
partition
echo -e "${Gre}Operation complete${Rcol}"
echo
echo -e "${Gre}Formatting partitions${RCol}"
format
echo -e "${Gre}Operation complete${Rcol}"
echo
echo -e "${Gre}Mounting partitions${RCol}"
mount_partitions
echo -e "${Gre}Operation complete${Rcol}"
echo
echo -e "${Gre}Copying files${RCol}"
copy_files
echo -e "${Gre}Operation complete${Rcol}"
echo
echo -e "${Gre}Unmounting${RCol}"
unmount_partitions
echo -e "${Gre}Operation complete${Rcol}"
echo
echo -e "${Gre}Setup completed successfully${RCol}"
