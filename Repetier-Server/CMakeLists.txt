# toplevel src CMake file

set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_subdirectory(json_spirit)
add_subdirectory(libconfig)
add_subdirectory(mongoose)
add_subdirectory(server)
add_subdirectory(moFileReader)

set(RepetierServer_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" ${JSON_SPIRIT_SRCS} ${MONGOOSE_SOURCES} ${MOFILEREADER_SOURCES} ${SERVER_SOURCES} ${LIBCONFIG_SOURCES} PARENT_SCOPE)
