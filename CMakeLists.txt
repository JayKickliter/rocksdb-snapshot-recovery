project(rocksdb-recovery-test)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)

find_package(RocksDB)

add_executable(rocksdb-recovery-test
  main.cc
  )

target_link_libraries(rocksdb-recovery-test
  RocksDB
  )
