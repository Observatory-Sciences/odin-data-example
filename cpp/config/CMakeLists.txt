# Copy files to build and substitute macros
configure_file(frameReceiver.sh .)
configure_file(frameProcessor.sh .)
configure_file(example-detector-fp.json .)
configure_file(example-detector-fr.json .)
#configure_file(frameSimulator.sh .)
configure_file(example-detector.cfg .)

# Install files generated in build by above commands, *not* directly from source
file(GLOB SCRIPTS ${CMAKE_CURRENT_BINARY_DIR}/*sh)
install(PROGRAMS ${SCRIPTS} DESTINATION example_detector)

# Install files generated in build by above commands, *not* directly from source
file(GLOB CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/*json)
install(FILES ${CONFIG_FILES} DESTINATION example_detector)

# Install files generated in build by above commands, *not* directly from source
file(GLOB CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/*cfg)
install(FILES ${CONFIG_FILES} DESTINATION example_detector)
