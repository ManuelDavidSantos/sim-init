#!/bin/bash

echo "Moving binaries to /bin"
mv ./sim-init /bin/
mv ./supervised /bin/
mv ./power-off /bin/

echo "Replacing init"
mv /sbin/init /sbin/init.runit
ln -s /bin/sim-init /sbin/init

echo "Creating log directory"
mkdir -p /var/log/services

echo "Adding service scripts"
mv ./core/* /etc/sim-init/
