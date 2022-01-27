cmake_minimum_required(VERSION 3.22)
project(webrtc-native-demo)

set(CMAKE_CXX_STANDARD 20)

# compiler version info
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}, version: "
        "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}, version: "
        "${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")

# lib and bin path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_BUILD_TYPE Release)

# import rtc
add_subdirectory(webrtc)

add_subdirectory(01_hello_rtc)
