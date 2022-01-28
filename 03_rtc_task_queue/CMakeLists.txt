
set(TARGET test_rtc_task_queue)

add_executable(${TARGET} main.cc)

target_link_libraries(${TARGET} webrtc)

