
set(SERVER test_rtc_network)
add_executable(${SERVER} main.cc)
target_link_libraries(${SERVER} webrtc)

