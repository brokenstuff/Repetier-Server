# Main CMake file

project(Repetier-Server)
cmake_minimum_required(VERSION 2.6)
 
# Appends the cmake/modules path inside the MAKE_MODULE_PATH variable which stores the
# directories of additional CMake modules (eg MacroOutOfSourceBuild.cmake):
set(CMAKE_MODULE_PATH ${Repetier-Server_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
 
# The macro below forces the build directory to be different from source directory:
include(MacroOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of source build.")

############## include Boost

# Activate debug if you don't know where cmake is looking for your boost files
set(Boost_DEBUG 1) 
SET(BOOST_INCLUDEDIR "C:/Boost/include/boost-1_52")
SET(BOOST_LIBRARYDIR "C:/Boost/lib") 
 
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.48.0 COMPONENTS filesystem thread program_options system date_time chrono regex)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
  #  target_link_libraries("Repetier-Server" ${Boost_LIBRARIES})
ENDIF()

add_subdirectory(Repetier-Server)
INCLUDE_DIRECTORIES("Repetier-Server/json_spirit")
INCLUDE_DIRECTORIES("Repetier-Server/mongoose")
INCLUDE_DIRECTORIES("Repetier-Server/moFileReader")
INCLUDE_DIRECTORIES("Repetier-Server/libconfig")
INCLUDE_DIRECTORIES("Repetier-Server/server")

#message("Files: ${RepetierServer_SOURCES}")
#message("Boost libs: ${Boost_LIBRARIES}")
add_executable("RepetierServer" ${RepetierServer_SOURCES})
target_link_libraries("RepetierServer" ${Boost_LIBRARIES})
IF (UNIX)
  target_link_libraries(RepetierServer dl m)
ENDIF (UNIX)
