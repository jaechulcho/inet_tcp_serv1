## cmake 최소 필요 버전
cmake_minimum_required(VERSION 2.8)

## 프로젝트명
project(serial_svr)

set(cpu_type cortex-a7)
set(fpu_type neon-vfpv4)
set(float_abi_type hard)

IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES arm)
  add_definitions(-pg -mcpu=${cpu_type} -mfpu=${fpu_type} -mfloat-abi=${float_abi_type} -pthread -std=c++11 -fPIC -ffast-math)
ELSE ()
  add_definitions(-std=c++11)
ENDIF()

#set(INCLUDE_DIRS "include" )
#set(VOXEL_LIBRARIES /usr/local/lib/libti3dtof.so /usr/local/lib/libvoxel.so)
include_directories("include")

add_executable(serial_svr serial_svr/inet_tcp_serv1.cpp)
add_executable(io_select io_select/io_select.cpp)

#target_include_directories(serial_svr ${INCLUDE_DIRS})
#target_link_libraries(tofapp X11 ${OpenCV_LIBS} ${VOXEL_LIBRARIES} pthread yaml-cpp rt )