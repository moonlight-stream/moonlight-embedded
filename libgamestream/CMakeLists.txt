find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(EXPAT REQUIRED)

pkg_check_modules(AVAHI REQUIRED avahi-client)

aux_source_directory(./ GAMESTREAM_SRC_LIST)

aux_source_directory(../third_party/moonlight-common-c/limelight-common GAMESTREAM_SRC_LIST)
aux_source_directory(../third_party/moonlight-common-c/limelight-common/OpenAES GAMESTREAM_SRC_LIST)

add_library(gamestream SHARED ${GAMESTREAM_SRC_LIST})
set_property(TARGET gamestream PROPERTY C_STANDARD 11)
set_target_properties(gamestream PROPERTIES SOVERSION 0 VERSION 2.1.0)

target_include_directories(gamestream PRIVATE ../third_party/moonlight-common-c ${AVAHI_INCLUDE_DIRS})
target_link_libraries (gamestream PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${EXPAT_LIBRARIES} ${AVAHI_LIBRARIES} ${CMAKE_DL_LIBS})

#install(TARGETS gamestream DESTINATION ${CMAKE_INSTALL_LIBDIR})