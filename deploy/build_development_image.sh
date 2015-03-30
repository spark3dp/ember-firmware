#!/bin/bash -e
# Script to generate development image

root_dir=$(cd $(dirname "$0"); pwd)

kernel_ver='3.8.13-bone71'

oib_config_file="${root_dir}/configs/smith-development.conf"
oib_common_config_file="${root_dir}/configs/smith-common.conf"
oib_temp_config_file="${root_dir}/configs/.smith-development.conf"

boot_partition_size_mb=50
boot_partition_size=$(($boot_partition_size_mb * 1024 * 1024))

# Devices to attach partitions to
loop0=/dev/loop0
loop1=/dev/loop1

# Mount points
boot_mount_point=/mnt/boot
root_mount_point=/mnt/root

Red='\e[0;31m'
Gre='\e[0;32m'
Yel='\e[0;33m'
RCol='\e[0m'

check_for_internet() {
  if ! ping -c 1 ftp.us.debian.org > /dev/null 2>&1; then
    echo -e "${Red}Could not reach ftp.us.debian.org, check internet connectivity, aborting${RCol}"
    exit 1
  fi
}

check_for_bc() {
  if ! which bc > /dev/null 2>&1; then
    echo -e "${Red}bc is required but not installed, install with 'apt-get install bc', aborting${RCol}"
    exit 1
  fi
}

cleanup() {
  if [ $? -ne 0 ]; then
    echo -e "${Red}Script exited with non-zero exit code, image not generated successfully${RCol}"
  fi
  rm -rf "${oib_temp_config_file}"
}

# Load the variables from the .project file put in place by RootStock-NG.sh to determine the name of the
# directory containing the resultant root filesystem
read_filesystem_name() {
  . "${root_dir}/.project"
  
  rootfs_dir="${root_dir}/deploy/${export_filename}/${deb_arch}-rootfs-${deb_distribution}-${deb_codename}"
  
  if [ ! -d "${rootfs_dir}" ]; then
    echo -e "${Red}could not find filesystem at ${rootfs_dir}, check ${root_dir}/.project file for \$export_filename, \$deb_arch, \$deb_distribution, \$deb_codename, aborting${RCol}"
    exit 1
  fi
}

# Create an empty image file sized to hold the root filesystem and boot partition
create_image_file() {
  # File name of resulting image
  img_file="${root_dir}/deploy/${export_filename}.img"
  
  # Calculate size based on filesystem size, boot parition size, and 10% extra for padding
  sum=$(($boot_partition_size + $rootfs_size))
  printf -v padding %.0f $(echo "$sum * 0.1" | bc)
  img_size=$(($sum + $padding))

  # Ensure no existing file
  rm -rf "${img_file}"

  # Create an empty file
  fallocate -l $img_size "${img_file}"

  ls -lh "${img_file}"
}

partition_image() {
  # sfdisk doesn't make the second partition fill the remainder of the image so fdisk is used
  # Create two partitions
  # The first is of type FAT16 with the bootable flag set and sized according to $boot_partition_size_mb
  # The second is of type Linux and sized to fill the remainder of the image
  echo -e "o\nn\np\n1\n\n+${boot_partition_size_mb}M\nt\ne\na\n1\nn\np\n2\n\n\nw" | fdisk "${img_file}"
}

detach_loopback_devices() {
  losetup -d "${loop0}"
  losetup -d "${loop1}"
}

attach_loopback_devices() {
  fdisk_out=$(fdisk -l "${img_file}")
  
  # Find the offsets of the partitions in sectors
  partition0_offset=$(echo "${fdisk_out}" | awk '$0 ~ partition {print $3}' partition="${img_file}1")
  partition1_offset=$(echo "${fdisk_out}" | awk '$0 ~ partition {print $2}' partition="${img_file}2")
  
  # Determine size of a sector
  sector_size=$(echo "${fdisk_out}" | sed -n 's/Units.*\b\([0-9]\+\).*bytes/\1/p')

  # Attach the partitions
  losetup "${loop0}" "${img_file}" --offset $(($partition0_offset * $sector_size)) --sizelimit $boot_partition_size
  losetup "${loop1}" "${img_file}" --offset $(($partition1_offset * $sector_size))
}

format() {
  mkfs.vfat -F 16 "${loop0}" -n boot
  mkfs.ext4 "${loop1}" -L root
}

ensure_unmounted() {
  # Make sure any existing partitions are unmounted
  umount "${loop0}" > /dev/null 2>&1 || true
  umount "${loop1}" > /dev/null 2>&1 || true
}

ensure_detached() {
  # Ensure that loopback devices are detached
  losetup -d "${loop0}" > /dev/null 2>&1 || true
  losetup -d "${loop1}" > /dev/null 2>&1 || true
}

mount_partitions() {
  mkdir -p "${boot_mount_point}"
  mkdir -p "${root_mount_point}"

  mount "${loop0}" "${boot_mount_point}"
  mount "${loop1}" "${root_mount_point}"
}

unmount_partitions() {
  umount "${loop0}"
  umount "${loop1}"

  rmdir "${boot_mount_point}"
  rmdir "${root_mount_point}"
}

copy_files() {
  # Copy root filesystem
  cp -r "${rootfs_dir}/"* "${root_mount_point}"

  # Copy the common boot files
  cp -r "${root_dir}/setup/boot/smith-common/${kernel_ver}/"* "${boot_mount_point}"

  # Copy the development specific boot files
  cp -r "${root_dir}/setup/boot/smith-development/${kernel_ver}/"* "${boot_mount_point}"
  sync
}

# Set up an exit handler to clean up temp files
trap cleanup EXIT

echo 'Smith development image builder script'
echo

check_for_bc

# The --skip-oib option can be specified to skip generating a root filesystem
# Whatever was last genereated with omap-image-builder will be used for the image
if [ "${1}" == '--skip-oib' ]; then
  echo -e "${Yel}Not generating root filesystem, using last generated filesystem${RCol}"
else
  check_for_internet
  
  # Combine the common and development options into a singe temp file
  cat "${oib_common_config_file}" "${oib_config_file}" > "${oib_temp_config_file}"

  echo -e "${Gre}Executing omap-image-builder for ${oib_config_file}  See ${root_dir}/oib.log for output${RCol}"
  cd "${root_dir}" && omap-image-builder/RootStock-NG.sh -c "${oib_temp_config_file}" > oib.log 2>&1
  # Read the resulting filesystem name
  echo -e "${Gre}Operation complete${RCol}"
fi

echo

read_filesystem_name
ensure_unmounted
ensure_detached

echo -e "${Gre}Calculating size of filesystem directory (${rootfs_dir})${RCol}"
# -s argument reports summary for specified directory
# --block-size=1 makes output size in bytes
rootfs_size=$(du -s --block-size=1 "${rootfs_dir}" | cut -f 1)
echo "filesystem is ${rootfs_size} bytes"
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Creating image file${RCol}"
create_image_file
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Partitioning image file${RCol}"
partition_image
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Attaching partitions as loopback devices${RCol}"
attach_loopback_devices
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
echo -e "${Gre}Detaching loopback devices${RCol}"
detach_loopback_devices
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Image generated successfully: ${img_file}${RCol}"
