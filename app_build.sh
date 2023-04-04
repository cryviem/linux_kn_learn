

rm -rf $PWD/test_app/out
mkdir $PWD/test_app/out && cd $PWD/test_app/out
cmake .. || exit 1
make
cd ../..