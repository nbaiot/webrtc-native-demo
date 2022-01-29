
set(SERVER rtc_tcp_server)
add_executable(${SERVER} server.cc)
target_link_libraries(${SERVER} webrtc)

set(CLIENT rtc_tcp_client)
add_executable(${CLIENT} client.cc)
target_link_libraries(${CLIENT} webrtc)
