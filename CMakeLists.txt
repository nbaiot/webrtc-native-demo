cmake_minimum_required(VERSION 3.22)
project(webrtc-native-demo)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Release)

# import rtc
add_subdirectory(webrtc)

add_subdirectory(01_hello_rtc)
