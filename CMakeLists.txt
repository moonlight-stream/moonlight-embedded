project(moonlight-embedded C)
cmake_minimum_required(VERSION 3.1)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

aux_source_directory(./src SRC_LIST)
list(APPEND SRC_LIST ./src/video/fake.c)

aux_source_directory(./third_party/moonlight-common-c/limelight-common SRC_LIST)
aux_source_directory(./third_party/moonlight-common-c/limelight-common/OpenAES SRC_LIST)

include_directories(./third_party/moonlight-common-c)

set(MOONLIGHT_DEFINITIONS)

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
pkg_check_modules(UDEV REQUIRED libudev)

if(BROADCOM_FOUND)
  aux_source_directory(./third_party/ilclient SRC_LIST)
  aux_source_directory(./third_party/h264bitstream SRC_LIST)
  list(APPEND SRC_LIST ./src/video/omx.c)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_OMX)
endif()

if(FREESCALE_FOUND)
  list(APPEND SRC_LIST ./src/video/imx.c)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_IMX)
endif()

add_executable(moonlight ${SRC_LIST})
set_property(TARGET moonlight PROPERTY C_STANDARD 11)

if(BROADCOM_FOUND)
  include_directories(./third_party/ilclient ./third_party/h264bitstream ${BROADCOM_INCLUDE_DIRS})
  target_link_libraries (moonlight PUBLIC ${BROADCOM_LIBRARIES})
  list(APPEND MOONLIGHT_DEFINITIONS ${BROADCOM_DEFINITIONS})
endif()

if(FREESCALE_FOUND)
  include_directories(${FREESCALE_INCLUDE_DIRS})
  target_link_libraries (moonlight PUBLIC ${FREESCALE_LIBRARIES})
endif()

set_property(TARGET moonlight PROPERTY COMPILE_DEFINITIONS ${MOONLIGHT_DEFINITIONS})
include_directories(./third_party/moonlight-common-c ${OPUS_INCLUDE_DIRS} ${EVDEV_INCLUDE_DIRS} ${AVAHI_INCLUDE_DIRS} ${UDEV_INCLUDE_DIRS})
target_link_libraries (moonlight PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${EVDEV_LIBRARIES} ${ALSA_LIBRARY} ${OPUS_LIBRARY} ${AVAHI_LIBRARIES} ${UDEV_LIBRARIES} ${CMAKE_DL_LIBS})

include(${CMAKE_ROOT}/Modules/GNUInstallDirs.cmake)
install(TARGETS moonlight DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY mappings DESTINATION ${CMAKE_INSTALL_DATADIR})
