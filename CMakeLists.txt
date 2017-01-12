cmake_minimum_required(VERSION 2.8.8)
project("ospray")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

# configure ospray stuff ---------------

# compiler
include(${PROJECT_SOURCE_DIR}/cmake/gcc.cmake)

# ----------------
set( APP_NAME "ospray-backend" )
MESSAGE( STATUS "application:" ${APP_NAME} )
MESSAGE( STATUS "binary output:" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )


# find ospray common -----
set( OSPRAY_SOURCE_DIR /zhome/academic/HLRS/zmc/zmcdkoer/ospr/install-sources/ospray )
set( OSPRAYCOMMON_INCLUDE_DIR ${OSPRAY_SOURCE_DIR} ) # this is to get common/affineSpace.h etc. Its weird since common is the ospray_common library...

# find OSPRAY ------
set( OSPRAY_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include )
set( OSPRAY_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib64/libospray.so
					  ${CMAKE_INSTALL_PREFIX}/lib64/libospray_common.so
					  ${CMAKE_INSTALL_PREFIX}/lib64/libospray_xml.so
					  ${CMAKE_INSTALL_PREFIX}/lib64/libospray_commandline.so)




#find_package(rserver REQUIRED)
#if (rserver_FOUND)
#    message("-- Found rserver version ${rserver_VERSION}: ${rserver_INCLUDE_DIRS}")
#else ()
#    message(FATAL_ERROR "rserver not found")
#endif ()


# ----------------------------------------------

# required to allow cmake to find findczmq.cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(czmq REQUIRED)
if (czmq_FOUND)
		message("-- Found czmq version ${czmq_VERSION}: ${czmq_INCLUDE_DIRS}")
else ()
		message(FATAL_ERROR "czmq not found")
endif ()


# JPEG_INCLUDE_DIR /zhome/academic/HLRS/zmc/zmcdkoer/ospr/install/include
# JPEG_LIBRARIES /zhome/academic/HLRS/zmc/zmcdkoer/ospr/install/lib/libjpeg.so
set(JPEG_INCLUDE_DIR /zhome/academic/HLRS/zmc/zmcdkoer/ospr/install/include )
set(JPEG_LIBRARIES /zhome/academic/HLRS/zmc/zmcdkoer/ospr/install/lib/libjpeg.so)
#find_package(JPEG)
#if (JPEG_FOUND)
#    message("-- Found jpeg version ${JPEG_VERSION}: ${JPEG_INCLUDE_DIR}")
#else ()
#    message(FATAL_ERROR "jpeg not found")
#endif ()

find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
	message( "ZLIB_INCLUDE_DIRS=${ZLIB_INCLUDE_DIRS}" )
	message( "ZLIB_LIBRARIES=${ZLIB_LIBRARIES}" )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif( ZLIB_FOUND )

# ----------------------------------------------


set( app_sources src/main.cpp
				 src/jpeg.cpp
				 src/string.cpp
				 src/rsi.cpp
				 src/gzstream/gzstream.cpp )

add_executable(${APP_NAME} ${app_sources} )

set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 14)


# additional include directories ---
include_directories( ${PROJECT_SOURCE_DIR}/include )

include_directories( ${OSPRAYCOMMON_INCLUDE_DIR} )
include_directories( ${OSPRAY_INCLUDE_DIR} )

# the libraries such as ospray_common or ospray_importer etc. get installed but their headers do not get installed
# therefore we will need to point it to the header locations in ospray source directory
include_directories( ${OSPRAY_SOURCE_DIR}/apps )

# moreover, other libraries such as ospray_commandline expect a different location again
include_directories( ${OSPRAY_SOURCE_DIR}/apps/common )

# ospray_cpp headers are installed into the ospray subdirectory within the ospray-include directory
include_directories( ${OSPRAY_INCLUDE_DIR}/ospray )

# additional libraries
target_link_libraries(${APP_NAME} ${OSPRAY_LIBRARIES} )
target_link_libraries(${APP_NAME} ${czmq_LIBRARIES} )
target_link_libraries(${APP_NAME} ${JPEG_LIBRARIES} )
target_link_libraries(${APP_NAME} ${ZLIB_LIBRARIES} )