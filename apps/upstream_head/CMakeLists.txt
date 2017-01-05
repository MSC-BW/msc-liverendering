


find_package(JPEG)
if (JPEG_FOUND)
    message("-- Found jpeg version ${JPEG_VERSION}: ${JPEG_INCLUDE_DIR}")
    message( "JPEG_INCLUDE_DIR=${JPEG_INCLUDE_DIR}" )
    message( "JPEG_LIBRARIES=${JPEG_LIBRARIES}" )
else ()
    message(FATAL_ERROR "jpeg not found")
endif ()

# upstream_head is the render client on the compute nodes =================================

# currently we dont compile rsi into a seperate library. instead we compile the source file directly with the project
add_executable(upstream_head main.cpp
							jpeg.cpp
							${CMAKE_CURRENT_SOURCE_DIR}/../../rsi/rsi.cpp)	
set_property(TARGET upstream_head PROPERTY CXX_STANDARD 11)
target_link_libraries(upstream_head ${czmq_LIBRARIES} )
target_link_libraries(upstream_head ${JPEG_LIBRARIES} )