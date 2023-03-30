project(choen_app)
cmake_minimum_required (VERSION 3.10)
find_package(Threads REQUIRED)

add_executable(choen_app 
    main.cpp)

target_include_directories(choen_app PUBLIC
    ../choen_dev)