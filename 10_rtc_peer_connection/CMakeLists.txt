
set(TARGET test_rtc_peer_connection)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc dl)

