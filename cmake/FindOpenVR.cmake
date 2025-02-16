# FindOpenVR.cmake
# Finds the OpenVR library and include directories
#
# This will define:
# OPENVR_FOUND - True if OpenVR was found
# OPENVR_INCLUDE_DIRS - OpenVR include directories
# OPENVR_LIBRARIES - OpenVR libraries

find_path(OPENVR_INCLUDE_DIR
    NAMES openvr.h openvr_driver.h
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        $ENV{OPENVR_ROOT_DIR}/headers
        $ENV{OPENVR_ROOT_DIR}/public/headers
        $ENV{OPENVR_ROOT_DIR}/include
    PATH_SUFFIXES
        openvr
)

find_library(OPENVR_LIBRARY
    NAMES openvr_api
    PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        /opt/local/lib
        $ENV{OPENVR_ROOT_DIR}/lib
        $ENV{OPENVR_ROOT_DIR}/lib/linux64
    PATH_SUFFIXES
        linux64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenVR DEFAULT_MSG OPENVR_LIBRARY OPENVR_INCLUDE_DIR)

if(OPENVR_FOUND)
    set(OPENVR_LIBRARIES ${OPENVR_LIBRARY})
    set(OPENVR_INCLUDE_DIRS ${OPENVR_INCLUDE_DIR})
endif()

mark_as_advanced(OPENVR_INCLUDE_DIR OPENVR_LIBRARY)