project(moonlight-embedded)
cmake_minimum_required(VERSION 3.2)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

aux_source_directory(./src SRC_LIST)

aux_source_directory(./common/limelight-common SRC_LIST)
aux_source_directory(./common/limelight-common/OpenAES SRC_LIST)

include_directories(./common)

find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(EXPAT REQUIRED)
find_package(ALSA REQUIRED)
find_package(Opus REQUIRED)
find_package(Broadcom)
find_package(Freescale)

find_package(PkgConfig REQUIRED)
pkg_check_modules(EVDEV REQUIRED libevdev)
pkg_check_modules(AVAHI REQUIRED avahi-client)

if(Broadcom_FOUND)
  aux_source_directory(./ilclient SRC_LIST)
  aux_source_directory(./h264bitstream SRC_LIST)

  include_directories(./ilclient ./h264bitstream ${BROADCOM_INCLUDE_DIRS})
  target_link_libraries (moonlight PUBLIC ${BROADCOM_LIBRARIES})
  set_property(TARGET moonlight PROPERTY COMPILE_DEFINITIONS ${BROADCOM_DEFINITIONS})
  list(APPEND SRC_LIST ./src/video/omx.c)
elseif(Freescale_FOUND)
  include_directories(${FREESCALE_INCLUDE_DIRS})
  target_link_libraries (moonlight PUBLIC ${FREESCALE_LIBRARIES})
  list(APPEND SRC_LIST ./src/video/imx.c)
else()
  list(APPEND SRC_LIST ./src/video/fake.c)
endif()

add_executable(moonlight ${SRC_LIST})
set_property(TARGET moonlight PROPERTY C_STANDARD 11)

include_directories(./moonlight-common-c ${OPUS_INCLUDE_DIRS} ${EVDEV_INCLUDE_DIRS} ${AVAHI_INCLUDE_DIRS})
target_link_libraries (moonlight PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${EVDEV_LIBRARIES} ${ALSA_LIBRARY} ${OPUS_LIBRARY} ${AVAHI_LIBRARIES})
