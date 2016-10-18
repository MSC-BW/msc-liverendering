



# downstream websocket client (remote user) ====================================

# currently we dont compile rsi into a seperate library. instead we compile the source file directly with the project
add_executable(downstream_wsclient main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../../rsi/rsi.cpp)	
set_property(TARGET downstream_wsclient PROPERTY CXX_STANDARD 11)

# todo: replace this with libpthread. we dont need czmq/zmq here but without linking czmq we get unresolved symbols from missing pthread
target_link_libraries(downstream_wsclient ${czmq_LIBRARIES} )