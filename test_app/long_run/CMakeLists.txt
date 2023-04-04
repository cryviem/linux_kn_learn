project(choen)
cmake_minimum_required (VERSION 3.10)
find_package(Threads REQUIRED)

add_executable(long_run 
    main.cpp)

target_include_directories(long_run PUBLIC
    ../../choen_rwnonblocking)