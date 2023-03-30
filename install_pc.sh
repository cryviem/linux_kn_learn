
if [ -z ${ACTIVE_PROJECT} ]
then
    echo "Not found active project, please run init_env.sh <project>"
    exit 1
fi
echo "START INSTALL PROJECT ${ACTIVE_PROJECT}"
# remove stale nodes
rm -f /dev/choen[0-1]
# uninstall existing module
rmmod choen
# install module, exit if fail
insmod $PWD/${ACTIVE_PROJECT}/choen.ko || exit 1
# get major number from recently installed module
major=`awk '/choen/ {print $1}' /proc/devices`
# create new nodes
mknod /dev/choen0 c $major 0
mknod /dev/choen1 c $major 1

chgrp choen /dev/choen[0-1]
chmod a+rw /dev/choen[0-1]