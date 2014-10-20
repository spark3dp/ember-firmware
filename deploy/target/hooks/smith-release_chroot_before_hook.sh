echo 'Log: in chroot before hook script'
echo 'Log: installing configuration files'
# Copy over config files
cp -rv ${DIR}/target/config/. ${tempdir}
