project(moonlight-embedded C)
cmake_minimum_required(VERSION 3.1)
set(MOONLIGHT_MAJOR_VERSION 0)
set(MOONLIGHT_MINOR_VERSION 1)
set(MOONLIGHT_PATCH_VERSION 0)
set(MOONLIGHT_VERSION ${MOONLIGHT_MAJOR_VERSION}.${MOONLIGHT_MINOR_VERSION}.${MOONLIGHT_PATCH_VERSION})

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(${CMAKE_ROOT}/Modules/GNUInstallDirs.cmake)

aux_source_directory(./src SRC_LIST)
aux_source_directory(./src/input SRC_LIST)

set(MOONLIGHT_DEFINITIONS)

find_package(ALSA REQUIRED)
find_package(Opus REQUIRED)
find_package(Broadcom)
find_package(Freescale)
find_package(CEC 3.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(EVDEV REQUIRED libevdev)
pkg_check_modules(UDEV REQUIRED libudev)
pkg_check_modules(SDL sdl2>=2.0.4)
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(AVUTIL libavutil)
pkg_check_modules(SWSCALE libswscale)

SET(SOFTWARE_FOUND AVCODEC_FOUND AND AVUTIL_FOUND AND SWSCALE_FOUND AND SDL_FOUND)
SET(MOONLIGHT_COMMON_INCLUDE_DIR ./third_party/moonlight-common-c)
SET(GAMESTREAM_INCLUDE_DIR ./libgamestream)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  list(APPEND SRC_LIST ./src/video/fake.c)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_FAKE LC_DEBUG)
elseif(NOT BROADCOM_FOUND AND NOT FREESCALE_FOUND AND NOT ${SOFTWARE_FOUND})
  message(FATAL_ERROR "No video output available")
endif()

if (${SOFTWARE_FOUND})
  list(APPEND SRC_LIST ./src/video/ffmpeg.c ./src/video/sdl.c ./src/audio/sdl.c)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_SDL)
endif()

if (BROADCOM_FOUND OR FREESCALE_FOUND OR CMAKE_BUILD_TYPE MATCHES Debug)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_EMBEDDED)
endif()

list(APPEND SRC_LIST ./src/audio/alsa.c)

add_subdirectory(libgamestream)

add_executable(moonlight ${SRC_LIST})
target_link_libraries(moonlight gamestream)
set_property(TARGET moonlight PROPERTY C_STANDARD 99)

if (CEC_FOUND AND CEC_VERSION_COMPATIBLE)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_LIBCEC)
  target_include_directories(moonlight PRIVATE ./third_party/libcec ${CEC_INCLUDE_DIRS}/libcec)
  target_link_libraries(moonlight ${CEC_LIBRARIES})
endif()

if(BROADCOM_FOUND)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_PI)
  aux_source_directory(./third_party/ilclient ILCLIENT_SRC_LIST)
  add_library(moonlight-pi SHARED ./src/video/pi.c ${ILCLIENT_SRC_LIST})
  target_include_directories(moonlight-pi PRIVATE ./third_party/ilclient ${BROADCOM_INCLUDE_DIRS} ${GAMESTREAM_INCLUDE_DIR} ${MOONLIGHT_COMMON_INCLUDE_DIR})
  target_link_libraries(moonlight-pi gamestream ${BROADCOM_LIBRARIES})
  set_property(TARGET moonlight-pi PROPERTY COMPILE_DEFINITIONS ${BROADCOM_DEFINITIONS})
  install(TARGETS moonlight-pi DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(FREESCALE_FOUND)
  list(APPEND MOONLIGHT_DEFINITIONS HAVE_IMX)
  add_library(moonlight-pi SHARED ./src/video/imx.c)
  target_include_directories(moonlight-imx PRIVATE ./third_party/ilclient ${FREESCALE_INCLUDE_DIRS} ${GAMESTREAM_INCLUDE_DIR} ${MOONLIGHT_COMMON_INCLUDE_DIR})
  target_link_libraries(moonlight-imx gamestream ${FREESCALE_LIBRARIES})
  install(TARGETS moonlight-imx DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if (${SOFTWARE_FOUND})
  target_include_directories(moonlight PRIVATE ${SDL_INCLUDE_DIRS} ${AVCODEC_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS} ${SWSCALE_INCLUDE_DIRS})
  target_link_libraries(moonlight ${SDL_LIBRARIES} ${AVCODEC_LIBRARIES} ${AVUTIL_LIBRARIES} ${SWSCALE_LIBRARIES})
endif()

set_property(TARGET moonlight PROPERTY COMPILE_DEFINITIONS ${MOONLIGHT_DEFINITIONS})
target_include_directories(moonlight PRIVATE ${GAMESTREAM_INCLUDE_DIR} ${MOONLIGHT_COMMON_INCLUDE_DIR} ${OPUS_INCLUDE_DIRS} ${EVDEV_INCLUDE_DIRS} ${UDEV_INCLUDE_DIRS})
target_link_libraries(moonlight ${EVDEV_LIBRARIES} ${ALSA_LIBRARY} ${OPUS_LIBRARY} ${UDEV_LIBRARIES} ${CMAKE_DL_LIBS})

add_subdirectory(docs)

install(TARGETS moonlight DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY mappings DESTINATION ${CMAKE_INSTALL_DATADIR})
install(FILES moonlight.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
