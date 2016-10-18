cmake_minimum_required(VERSION 2.8.8)
project("zmq-test")



# required to allow cmake to find findczmq.cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(czmq REQUIRED)
if (czmq_FOUND)
    message("-- Found czmq version ${czmq_VERSION}: ${czmq_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "czmq not found")
endif ()

include_directories( ${CMAKE_INSTALL_PREFIX}/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty )

add_subdirectory(apps)

#add_executable(zmq-test-server main_server.cpp )	
#set_property(TARGET zmq-test-server PROPERTY CXX_STANDARD 11)

# required to allow including from custom install directory
#include_directories( ${CMAKE_INSTALL_PREFIX}/include )
#target_link_libraries(zmq-test-server ${czmq_LIBRARIES} )

# upstream client ====================================
#add_executable(zmq-test-client main_client.cpp si.cpp )
#set_property(TARGET zmq-test-client PROPERTY CXX_STANDARD 11)

# required to allow including from custom install directory
#include_directories( ${CMAKE_INSTALL_PREFIX}/include )
#INCLUDE_DIRECTORIES ( ${CMAKE_CURRENT_SOURCE_DIR}/include )
#target_link_libraries(zmq-test-client ${czmq_LIBRARIES} )

# downstream client ====================================
#add_executable(zmq-test-client-downstream main_client_downstream.cpp si.cpp )
#set_property(TARGET zmq-test-client-downstream PROPERTY CXX_STANDARD 11)

# required to allow including from custom install directory
#include_directories( ${CMAKE_INSTALL_PREFIX}/include )
#INCLUDE_DIRECTORIES ( ${CMAKE_CURRENT_SOURCE_DIR}/include )
#target_link_libraries(zmq-test-client-downstream ${czmq_LIBRARIES} )
