
# remove stale nodes
rm -f /dev/choen[0-1]
# uninstall existing module
rmmod choen
# install module, exit if fail
insmod $PWD/choen_dev/choen.ko || exit 1
# get major number from recently installed module
major=`awk '/choen/ {print $1}' /proc/devices`
# create new nodes
mknod /dev/choen0 c $major 0
mknod /dev/choen1 c $major 1

chgrp choen /dev/choen[0-1]
chmod g+rw /dev/choen[0-1]