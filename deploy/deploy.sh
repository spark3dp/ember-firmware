#!/bin/bash
ssh_opts="-o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o LogLevel=quiet"
scp ${ssh_opts} initramfs/init root@192.168.7.2:/usr/share/initramfs-tools/
ssh ${ssh_opts} root@192.168.7.2 'bash -s' <<'ENDSSH'
update-initramfs -u
cp -v /boot/initrd.img-$(uname -r) /media/boot/initrd.img
ENDSSH
