cmake_minimum_required(VERSION 2.8.8)
project("zmq-test")


option(BUILD_PROXY "build proxy" OFF)
option(BUILD_UPSTREAM_HEAD "build upstream client" OFF)
option(BUILD_DOWNSTREAM_WSCLIENT "build downstream websocket client" OFF)


if( ${BUILD_PROXY} OR ${BUILD_UPSTREAM_HEAD} )
	# required to allow cmake to find findczmq.cmake
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
	find_package(czmq REQUIRED)
	if (czmq_FOUND)
	    message("-- Found czmq version ${czmq_VERSION}: ${czmq_INCLUDE_DIRS}")
	else ()
	    message(FATAL_ERROR "czmq not found")
	endif ()
endif()

include_directories( ${CMAKE_INSTALL_PREFIX}/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty )

add_subdirectory(apps)

