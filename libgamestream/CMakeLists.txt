find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(EXPAT REQUIRED)

pkg_check_modules(AVAHI REQUIRED avahi-client)

aux_source_directory(./ GAMESTREAM_SRC_LIST)
aux_source_directory(../third_party/h264bitstream GAMESTREAM_SRC_LIST)

aux_source_directory(../third_party/moonlight-common-c/limelight-common MOONLIGHT_COMMON_SRC_LIST)
aux_source_directory(../third_party/moonlight-common-c/limelight-common/OpenAES MOONLIGHT_COMMON_SRC_LIST)

add_library(moonlight-common SHARED ${MOONLIGHT_COMMON_SRC_LIST})

add_library(gamestream SHARED ${GAMESTREAM_SRC_LIST})
set_property(TARGET gamestream PROPERTY C_STANDARD 11)
target_link_libraries(gamestream LINK_PUBLIC moonlight-common)

set_target_properties(gamestream PROPERTIES SOVERSION 0 VERSION ${MOONLIGHT_VERSION})
set_target_properties(moonlight-common PROPERTIES SOVERSION 0 VERSION ${MOONLIGHT_VERSION})

target_include_directories(gamestream PRIVATE ../third_party/moonlight-common-c ../third_party/h264bitstream ${AVAHI_INCLUDE_DIRS})
target_link_libraries (gamestream PUBLIC ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${AVAHI_LIBRARIES})

target_link_libraries (gamestream PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

install(TARGETS gamestream moonlight-common DESTINATION ${CMAKE_INSTALL_LIBDIR})
