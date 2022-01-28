
set(TARGET test_rtc_sigslot)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc)

