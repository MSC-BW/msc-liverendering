



# upstream_head is the render client on the compute nodes =================================

# currently we dont compile rsi into a seperate library. instead we compile the source file directly with the project
add_executable(upstream_head main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../rsi/rsi.cpp)	
set_property(TARGET upstream_head PROPERTY CXX_STANDARD 11)
target_link_libraries(upstream_head ${czmq_LIBRARIES} )