

# proxy-server ====================================
if( ${BUILD_PROXY} )
	add_subdirectory( proxy )
endif()

# upstream client (renderer head node) ====================================
if( ${BUILD_UPSTREAM_HEAD} )
	add_subdirectory( upstream_head )
endif()

# downstream websocket client (remote user) ====================================
if( ${BUILD_DOWNSTREAM_WSCLIENT} )
	add_subdirectory( downstream_wsclient )
endif()