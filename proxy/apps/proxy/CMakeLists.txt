

# proxy-server ====================================
add_executable(proxy main.cpp )	
set_property(TARGET proxy PROPERTY CXX_STANDARD 11)
target_link_libraries(proxy ${czmq_LIBRARIES} )