
# proxy-server ====================================
add_subdirectory( proxy )

# upstream client (renderer head node) ====================================
add_subdirectory( upstream_head )

# downstream websocket client (remote user) ====================================
add_subdirectory( downstream_wsclient )