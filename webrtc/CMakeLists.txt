
add_library(webrtc STATIC IMPORTED GLOBAL)

set(rtc_include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/rtc
  ${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/third_party/abseil-cpp
)

set(rtc_depend_libs
  -pthread
  -latomic
)

set(rtc_defines
  WEBRTC_POSIX
  WEBRTC_LINUX
)

set_target_properties(webrtc PROPERTIES
  IMPORTED_LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/lib/libwebrtc.a
  INTERFACE_INCLUDE_DIRECTORIES "${rtc_include}"
  INTERFACE_LINK_LIBRARIES "${rtc_depend_libs}"
  INTERFACE_COMPILE_DEFINITIONS "${rtc_defines}"
)
