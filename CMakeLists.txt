cmake_minimum_required(VERSION 3.1)
project(moonlight-embedded VERSION 2.4.6 LANGUAGES C)
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
SET(CMAKE_C_STANDARD 99)
include(${CMAKE_ROOT}/Modules/GNUInstallDirs.cmake)

aux_source_directory(./src SRC_LIST)

set(MOONLIGHT_DEFINITIONS)

find_package(Opus REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL sdl2>=2.0.4)
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(AVUTIL libavutil)

SET(MOONLIGHT_COMMON_INCLUDE_DIR ./third_party/moonlight-common-c/src)
SET(GAMESTREAM_INCLUDE_DIR ./libgamestream)

include_directories("${PROJECT_BINARY_DIR}")

add_subdirectory(libgamestream)

add_executable(moonlight ${SRC_LIST})
target_link_libraries(moonlight gamestream)

target_sources(moonlight PRIVATE ./src/video/ffmpeg.c)
target_include_directories(moonlight PRIVATE ${AVCODEC_INCLUDE_DIRS} ${AVUTIL_INCLUDE_DIRS})
target_link_libraries(moonlight ${AVCODEC_LIBRARIES} ${AVUTIL_LIBRARIES})

target_include_directories(moonlight PRIVATE "/usr/local/opt/openssl/include")

if(SDL_FOUND)
    list(APPEND MOONLIGHT_DEFINITIONS HAVE_SDL)
    list(APPEND MOONLIGHT_OPTIONS SDL)
    target_sources(moonlight PRIVATE ./src/video/sdl.c ./src/audio/sdl.c ./src/input/sdl.c)
    target_include_directories(moonlight PRIVATE ${SDL_INCLUDE_DIRS})
    target_link_libraries(moonlight ${SDL_LIBRARIES})
endif()

configure_file("./src/configuration.h.in" "${PROJECT_BINARY_DIR}/configuration.h")

set_property(TARGET moonlight PROPERTY COMPILE_DEFINITIONS ${MOONLIGHT_DEFINITIONS})
target_include_directories(moonlight PRIVATE ${GAMESTREAM_INCLUDE_DIR} ${MOONLIGHT_COMMON_INCLUDE_DIR} ${OPUS_INCLUDE_DIRS})
target_link_libraries(moonlight ${OPUS_LIBRARY} ${CMAKE_DL_LIBS})

add_subdirectory(docs)

install(TARGETS moonlight DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES gamecontrollerdb.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/moonlight)
install(FILES moonlight.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
