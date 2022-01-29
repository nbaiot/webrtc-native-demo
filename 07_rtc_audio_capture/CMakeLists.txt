
set(TARGET test_rtc_audio_capture)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc -ldl)

