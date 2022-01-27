
set(TARGET hello_rtc)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc)

