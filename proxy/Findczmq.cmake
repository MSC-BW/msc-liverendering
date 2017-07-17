include(FindPkgConfig)
PKG_CHECK_MODULES(PC_ZEROMQ "libczmq")

find_path(
    czmq_INCLUDE_DIRS
    NAMES czmq.h
    HINTS ${PC_czmq_INCLUDE_DIRS}
)

find_library(
    czmq_LIBRARIES
    NAMES czmq
    HINTS ${PC_czmq_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(czmq DEFAULT_MSG czmq_LIBRARIES czmq_INCLUDE_DIRS)
mark_as_advanced(czmq_LIBRARIES czmq_INCLUDE_DIRS)
