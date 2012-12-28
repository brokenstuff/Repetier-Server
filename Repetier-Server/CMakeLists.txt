# toplevel src CMake file

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost 1.45.0 COMPONENTS filesystem thread program_options system)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

add_subdirectory(json_spirit)
add_subdirectory(libconfig)
add_subdirectory(mongoose)
add_subdirectory(server)

set(Repetier-Server_SOURCES main.cpp ${JSON_SPIRIT_SRCS} ${MONGOOSE_SOURCES} ${SERVER_SOURCES} ${LIBCONFIG_SOURCES})
