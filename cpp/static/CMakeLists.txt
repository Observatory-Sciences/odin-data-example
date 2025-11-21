# Copy files to build and substitute macros
configure_file(index.html .)

# Install files generated in build by above commands, *not* directly from source
file(GLOB CONFIG_FILES ${CMAKE_CURRENT_BINARY_DIR}/*html)
install(FILES ${CONFIG_FILES} DESTINATION example_detector/static)

# Install the static support files
install(DIRECTORY js DESTINATION example_detector/static)
install(DIRECTORY img DESTINATION example_detector/static)
