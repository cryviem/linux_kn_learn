# Add more project to this list
list="choen_base choen_rwblocking"

if [ -z $1 ]
then
    echo "No project specified"
    exit 1
fi

cmp=`echo ${list} | grep -w -o $1`
if [ -z ${cmp} ]
then
    echo "project not found"
    exit 1
fi
echo "Set active project: ${cmp}"
export ACTIVE_PROJECT=${cmp}