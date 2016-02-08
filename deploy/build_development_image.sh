#!/bin/bash -e
# Script to generate development image

root_dir=$(cd $(dirname "$0"); pwd)

oib_config_file="${root_dir}/configs/smith-development.conf"
oib_common_config_file="${root_dir}/configs/smith-common.conf"
oib_temp_config_file="${root_dir}/configs/.smith-development.conf"

clone_oib_script="${root_dir}/build_scripts/clone_oib.sh"

install_script="${root_dir}/build_scripts/install.sh"

bootloader_dir="${root_dir}/setup/u-boot"

# Device to attach partition to
loop0=/dev/loop0

# Mount point
root_mount_point=/mnt/root

# Additional space in MB (to add extra space to the partition without requiring a resize after flashing)
additional_space_MB=0

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
  source "${root_dir}/.project"
  
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

  # Calculate size based on filesystem size and 10% extra for padding
  printf -v total_size_B %.0f $(echo "1.1 * $rootfs_size_B + $additional_space_MB * 1024 * 1024" | bc)

  # Using file as loop device requires size to be a multiple of 512
  printf -v img_size_B %.0f $(echo "$total_size_B + (512 - ($total_size_B % 512))" | bc)

  # Ensure no existing file
  rm -rf "${img_file}"

  # Create an empty file
  fallocate -l $img_size_B "${img_file}"

  ls -lh "${img_file}"
}

partition_image() {
  # Create partition of type Linux and sized to fill the remainder of the image
  echo -e "o\nn\np\n1\n2048\n\nt\n83\nw" | fdisk "${img_file}"
}

detach_loopback_device() {
  losetup -d "${loop0}"
}

attach_loopback_device() {
  fdisk_out=$(fdisk -l "${img_file}")
  
  # Find the offsets of the partition in sectors
  partition0_offset=$(echo "${fdisk_out}" | awk '$0 ~ partition {print $2}' partition="${img_file}1")
  
  # Determine size of a sector
  sector_size=$(echo "${fdisk_out}" | sed -n 's/Units.*\b\([0-9]\+\).*bytes/\1/p')

  # Attach the partition
  losetup "${loop0}" "${img_file}" --offset $(($partition0_offset * $sector_size))
}

format() {
  mkfs.ext4 "${loop0}" -L root
}

ensure_unmounted() {
  # Make sure any existing partitions are unmounted
  umount "${loop0}" > /dev/null 2>&1 || true
}

ensure_detached() {
  # Ensure that loopback device is detached
  losetup -d "${loop0}" > /dev/null 2>&1 || true
}

mount_partition() {
  mkdir -p "${root_mount_point}"

  mount "${loop0}" "${root_mount_point}"
}

unmount_partition() {
  umount "${loop0}"

  rmdir "${root_mount_point}"
}

copy_root_filesystem() {
  # Copy root filesystem
  cp --archive "${rootfs_dir}/"* "${root_mount_point}"
  sync
}

confirm_version() {
  firmware_version=$(smith-config version)
  echo -e "${Yel}Currently deployed version of smith ruby gem: ${firmware_version}${RCol}"
  echo -ne "${Yel}Is this the version of the firmware you want to incorporate into the image and are the correct versions of the other components of the firmware currently built on this system? [y/N] ${RCol}"
  read -r response
  echo
  case $response in
    [yY][eE][sS]|[yY]) 
      # confirmed
      ;;
    *)
      echo -e "${Red}Install correct version of firmware and start again, aborting${RCol}"
      exit 1
      ;;
  esac
}

install_deploy_directory() {
  # Download a fresh copy of the release source code
  # GitHub generates these archives when we tag a commit as a release
  local temp_dir=$(mktemp -d)
  if ! wget -P "${temp_dir}" "${source_url}"; then
    echo -e "${Red}Unable to download release source code, ensure that the correct tag exists in the git repository and that the url is valid ("${source_url}"), aborting${RCol}"
    exit 1
  fi
  tar zxf "${temp_dir}/${firmware_version}.tar.gz" -C "${temp_dir}"
  cp -rv "${temp_dir}/ember-firmware-${firmware_version}/deploy" "${rootfs_dir}/root"
}

install_release_image() {
  # Download the release image
  # Use the download to populate the setup directory for the emmc setup script
  local temp_dir=$(mktemp -d)
  local firmware_dir="${rootfs_dir}/root/deploy/setup/main/firmware"
  if ! wget -P "${temp_dir}" "${image_url}"; then
    echo -e "${Red}Unable to download release image, ensure that the url ("${image_url}") is valid, aborting${RCol}"
    exit 1
  fi
  tar xf "${temp_dir}/smith-${firmware_version}.tar" -C "${temp_dir}"
  mkdir -pv  "${firmware_dir}"
  cp -v "${temp_dir}/smith-${firmware_version}.img" "${firmware_dir}"
  cp -v "${temp_dir}/md5sum" "${firmware_dir}/versions"
}

validate_url() {
  # Check if the specified url is valid (HEAD returns 2xx or 3xx status code)
  if ! curl -s --head "${1}" | head -n 1 | grep "HTTP/1.[01] [23].." > /dev/null; then
    echo -e "${Red}${1} does not point to an existing HTTP resource, aborting${RCol}"
    exit 1
  fi
}

write_bootloader() {
  # attach the image file to a loopback device, this time without an offset
  losetup "${loop0}" "${img_file}"

  # see https://eewiki.net/display/linuxonarm/BeagleBone+Black#BeagleBoneBlack-SetupmicroSDcard
  dd if="${bootloader_dir}/MLO" of="${loop0}" count=1 seek=1 bs=128k
  dd if="${bootloader_dir}/u-boot.img" of="${loop0}" count=2 seek=1 bs=384k

  detach_loopback_device
}

# Set up an exit handler to clean up temp files
trap cleanup EXIT

echo 'Smith development image builder script'
echo

check_for_bc
check_for_internet
confirm_version

# This is where the script downloads the release source code from to get the deploy directory
source_url="https://github.com/spark3dp/ember-firmware/archive/${firmware_version}.tar.gz"

# This is where the script downloads the release image from to populate the setup directory with firmware files for flashing emmc
image_url="https://s3.amazonaws.com/printer-firmware/smith-${firmware_version}.tar"

# Check that the urls are valid before spending a lot of time generating a file system
validate_url "${source_url}"
validate_url "${image_url}"

# The --skip-oib option can be specified to skip generating a root filesystem
# Whatever was last genereated with omap-image-builder will be used for the image
if [ "${1}" == '--skip-oib' ]; then
  echo -e "${Yel}Not generating root filesystem, using last generated filesystem${RCol}"
else
  echo -e "${Gre}Cloning omap-image-builder${RCol}"
  cd "${root_dir}" && "${clone_oib_script}"
  echo -e "${Gre}Operation complete${RCol}"
  echo
  
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

echo -e "${Gre}Installing firmware${RCol}"
"${install_script}" "${rootfs_dir}"
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Installing deploy directory${RCol}"
install_deploy_directory
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Installing release image into deploy setup directory${RCol}"
install_release_image
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Calculating size of filesystem directory (${rootfs_dir})${RCol}"
# -s argument reports summary for specified directory
# --block-size=1 makes output size in bytes
rootfs_size_B=$(du -s --block-size=1 "${rootfs_dir}" | cut -f 1)
echo "filesystem is ${rootfs_size_B} bytes"
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
echo -e "${Gre}Attaching partition as loopback device${RCol}"
attach_loopback_device
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Formatting partition${RCol}"
format
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Mounting partition${RCol}"
mount_partition
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Copying root filesystem${RCol}"
copy_root_filesystem
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Unmounting${RCol}"
unmount_partition
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Detaching loopback device${RCol}"
detach_loopback_device
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Writing bootloader to image${RCol}"
write_bootloader
echo -e "${Gre}Operation complete${RCol}"
echo
echo -e "${Gre}Image generated successfully: ${img_file}${RCol}"
