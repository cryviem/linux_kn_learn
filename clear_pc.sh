
if [ -z ${ACTIVE_PROJECT} ]
then
    echo "Not found active project, please run init_env.sh <project>"
    exit 1
fi
echo "START CLEAR PROJECT ${ACTIVE_PROJECT}"
make -C /lib/modules/`uname -r`/build M=$PWD/${ACTIVE_PROJECT} clean