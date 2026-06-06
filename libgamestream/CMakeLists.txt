set(SO_VERSION 4)

find_package(LibUUID REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL 1.1.0 REQUIRED)
find_package(EXPAT REQUIRED)

if (NOT HAIKU)
pkg_check_modules(AVAHI REQUIRED avahi-client)
endif()

aux_source_directory(./ GAMESTREAM_SRC_LIST)
aux_source_directory(../third_party/h264bitstream GAMESTREAM_SRC_LIST)

aux_source_directory(../third_party/moonlight-common-c/enet MOONLIGHT_COMMON_SRC_LIST)
aux_source_directory(../third_party/moonlight-common-c/src MOONLIGHT_COMMON_SRC_LIST)
aux_source_directory(../third_party/moonlight-common-c/reedsolomon MOONLIGHT_COMMON_SRC_LIST)

# Build shared library by default, but allows user override
if (NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
  set(BUILD_SHARED_LIBS_OVERRIDE ON)
endif()

add_library(moonlight-common ${MOONLIGHT_COMMON_SRC_LIST})
add_library(gamestream ${GAMESTREAM_SRC_LIST})

if (BUILD_SHARED_LIBS_OVERRIDE)
  unset(BUILD_SHARED_LIBS)
  unset(BUILD_SHARED_LIBS_OVERRIDE)
endif()

target_link_libraries(gamestream moonlight-common)

set_target_properties(gamestream PROPERTIES SOVERSION ${SO_VERSION} VERSION ${PROJECT_VERSION})
set_target_properties(moonlight-common PROPERTIES SOVERSION ${SO_VERSION} VERSION ${PROJECT_VERSION})

if(HAIKU)
    target_include_directories(gamestream PRIVATE ../third_party/moonlight-common-c/src ../third_party/h264bitstream ${LibUUID_INCLUDE_DIRS})
    target_link_libraries(gamestream ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${LibUUID_LIBRARIES})
else()
    target_include_directories(gamestream PRIVATE ../third_party/moonlight-common-c/src ../third_party/h264bitstream ${AVAHI_INCLUDE_DIRS} ${LibUUID_INCLUDE_DIRS})
    target_link_libraries(gamestream ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${AVAHI_LIBRARIES} ${LibUUID_LIBRARIES})
endif()

target_include_directories(moonlight-common PRIVATE ../third_party/moonlight-common-c/reedsolomon ../third_party/moonlight-common-c/enet/include)

target_link_libraries(gamestream ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

install(TARGETS gamestream moonlight-common DESTINATION ${CMAKE_INSTALL_LIBDIR})
