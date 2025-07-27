# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(ftctimer)

# Run the Python script to get the version from package.json
execute_process(
    COMMAND ${CMAKE_COMMAND} -E env python ${CMAKE_SOURCE_DIR}/get_version.py ${CMAKE_BINARY_DIR}/project_version.cmake
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Include the generated version file
include(${CMAKE_BINARY_DIR}/project_version.cmake)

# Add custom command to run the update_version.py script
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/main/version.h
    COMMAND ${CMAKE_COMMAND} -E env python ${CMAKE_SOURCE_DIR}/get_version.py ${CMAKE_BINARY_DIR}/project_version.cmake
    DEPENDS ${CMAKE_SOURCE_DIR}/package.json
    COMMENT "Updating version.h from package.json"
)

# Ensure version.h is generated before building
add_custom_target(update_version DEPENDS ${CMAKE_SOURCE_DIR}/main/version.h)

# Specify custom linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/ota.ld)

# Define spiffs partition
spiffs_create_partition_image(spiffs spiffs FLASH_IN_PROJECT)

# Custom command to create the spiffs.bin
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/spiffs.bin
    COMMAND mkspiffs -c ${CMAKE_SOURCE_DIR}/spiffs -b 4096 -p 256 -s 0x100000 ${CMAKE_BINARY_DIR}/spiffs.bin
    DEPENDS ${CMAKE_SOURCE_DIR}/spiffs
    COMMENT "Creating SPIFFS image"
)

# Custom target to ensure spiffs.bin is created
add_custom_target(create_spiffs_image ALL DEPENDS ${CMAKE_BINARY_DIR}/spiffs.bin)

# Ensure the spiffs_image is created before building the project
add_dependencies(${PROJECT_NAME}.elf create_spiffs_image)

# Ensure the version header is generated before building
add_dependencies(${PROJECT_NAME}.elf update_version)

include_directories(${CMAKE_SOURCE_DIR}/main)