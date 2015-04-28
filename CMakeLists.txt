project(moonlight-embedded)
cmake_minimum_required(VERSION 3.2)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

aux_source_directory(./common/limelight-common SRC_LIST)
aux_source_directory(./common/limelight-common/OpenAES SRC_LIST)
aux_source_directory(./src SRC_LIST)

include_directories(./common)

add_executable(moonlight ${SRC_LIST})
set_property(TARGET moonlight PROPERTY C_STANDARD 11)

find_package(Threads)
find_package(CURL)
find_package(OpenSSL)
find_package(EXPAT)
find_package(ALSA)
find_package(Opus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(EVDEV libevdev)

include_directories(./moonlight-common-c ${OPUS_INCLUDE_DIRS} ${EVDEV_INCLUDE_DIRS})
target_link_libraries (moonlight PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${EVDEV_LIBRARIES} ${ALSA_LIBRARY} ${OPUS_LIBRARY})
