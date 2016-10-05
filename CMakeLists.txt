cmake_minimum_required(VERSION 2.8.8)
project("zmq-test")


#find_package(PkgConfig)
#pkg_check_modules(test libczmq)

# required to allow cmake to find findczmq.cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(czmq REQUIRED)

if (czmq_FOUND)
    message("-- Found czmq version ${czmq_VERSION}: ${czmq_INCLUDE_DIRS}")
else ()
    message(FATAL_ERROR "czmq not found")
endif ()


add_executable(zmq-test-server main_server.cpp )

# required to allow including from custom install directory
include_directories( ${CMAKE_INSTALL_PREFIX}/include )
target_link_libraries(zmq-test-server ${czmq_LIBRARIES} )

add_executable(zmq-test-client main_client.cpp )

# required to allow including from custom install directory
include_directories( ${CMAKE_INSTALL_PREFIX}/include )
target_link_libraries(zmq-test-client ${czmq_LIBRARIES} )