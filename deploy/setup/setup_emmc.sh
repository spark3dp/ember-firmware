#!/bin/bash -e

kernel_ver='3.8.13-bone63'
disk=/dev/mmcblk1
setup_dir=$(cd $(dirname "$0"); pwd)

echo 'eMMC setup script'
echo "Using boot files for kernel version ${kernel_ver}"
printf "The target device is ${disk}. Make sure this is correct as it will be erased. Check lsblk if unsure. Type in yes to continue: "

unset confirm
read confirm
if [[ "${confirm}" != 'yes' ]]; then
  echo 'aborting'
  exit
fi

# Make sure any existing partitions are unmounted
umount "${disk}p1" > /dev/null 2>&1 || true
umount "${disk}p2" > /dev/null 2>&1 || true

echo
echo "Partitioning ${disk}"

sfdisk --in-order --Linux --unit M ${disk} <<-__EOF__
1,50,0xE,*
,,,-
__EOF__

echo
echo 'Formatting partitions'

mkfs.vfat -F 16 "${disk}p1" -n boot
mkfs.ext4 "${disk}p2" -L main

echo 'Mounting partitions'

mkdir -p /mnt/boot
mkdir -p /mnt/main
mount "${disk}p1" /mnt/boot
mount "${disk}p2" /mnt/main

echo
echo 'Copying files'

cp -r "${setup_dir}/boot/${kernel_ver}/"* /mnt/boot
cp -r "${setup_dir}/main/"* /mnt/main

echo
echo 'Unmounting'

umount /mnt/boot
umount /mnt/main
rmdir /mnt/boot
rmdir /mnt/main

echo
echo 'Done'
