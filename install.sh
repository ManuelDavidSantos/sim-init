#!/bin/bash

echo "Moving binaries to /bin"

chmod +x sim-init
chmod +x supervised
chmod +x power-off
chmod +x re-boot

mv ./sim-init /bin/
mv ./supervised /bin/
mv ./power-off /bin/
mv ./re-boot/bin/

echo "Replacing init"
mv /sbin/init /sbin/init.runit
ln -s /bin/sim-init /sbin/init

if [ -f /bin/reboot ]; then
  mv /bin/reboot /bin/reboot.old
  ln -s /bin/re-boot /bin/reboot
fi

if [ -f /bin/poweroff]; then
  mv /bin/poweroff /bin/poweroff.old
  ln -s /bin/power-off /bin/poweroff
fi

echo "Creating service directory"
mkdir -p /etc/service

echo "Creating log directory"
mkdir -p /var/log/services
