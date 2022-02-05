
set(TARGET test_rtc_stun)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc)
