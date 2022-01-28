
set(TARGET test_platform_thread)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc)

