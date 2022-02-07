#find_package(Boost 1.78 COMPONENTS regex system REQUIRED)
find_package(Boost 1.8 REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(TARGET rtc_janus_clinet)

aux_source_directory(. SRCS)

add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} webrtc)

