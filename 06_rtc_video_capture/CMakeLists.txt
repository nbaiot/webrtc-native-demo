
set(TARGET test_rtc_video_capture)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc)

