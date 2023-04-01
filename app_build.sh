

rm -rf $PWD/test_app/choen_base/out
mkdir $PWD/test_app/choen_base/out && cd $PWD/test_app/choen_base/out
cmake .. || exit 1
make
cd ../../..