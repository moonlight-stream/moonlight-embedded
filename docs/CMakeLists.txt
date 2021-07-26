add_custom_command(OUTPUT moonlight.1 COMMAND pod2man --section=1 --center="Moonlight Embedded Manual" --name="MOONLIGHT" --release="moonlight 2.5.0" ${CMAKE_CURRENT_SOURCE_DIR}/README.pod > moonlight.1)
add_custom_target(docs ALL DEPENDS moonlight.1)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/moonlight.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
