

rm -rf $PWD/choen_test_app/out
mkdir $PWD/choen_test_app/out && cd $PWD/choen_test_app/out
cmake .. || exit 1
make
cd ../..