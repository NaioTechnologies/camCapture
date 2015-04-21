#-------------------------------------------------------------------------------
#
#   LIBVITALS Module finder.
#
#   LIBJSONCPP_FOUND           - system has libjsoncpp
#   LIBJSONCPP_INCLUDE_DIRS    - the libjsoncpp include directory
#   LIBJSONCPP_LIBRARIES       - Link these to use libjsoncpp
#   LIBJSONCPP_DEFINITIONS     - Compiler switches required for using libjsoncpp
#   LIBJSONCPP_VERSION         - libjsoncpp version
#
#-------------------------------------------------------------------------------

if( JSONCPP_LIBRARIES AND JSONCPP_INCLUDE_DIRS )
    set( JSONCPP_FOUND true )
else( JSONCPP_LIBRARIES AND JSONCPP_INCLUDE_DIRS )

    # Looking for include folders
    find_path( JSONCPP_INCLUDE_DIR
        NAMES
            json/json.h
        PATHS
            /usr/include
            /usr/local/include
        PATH_SUFFIXES
            jsoncpp
    )
    SET( JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR} )

    # Looking for library folders
    find_library( JSONCPP_LIBRARY
        NAMES
            jsoncpp
        HINTS
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
    )
    SET( JSONCPP_LIBRARIES ${JSONCPP_LIBRARY} )

    if( JSONCPP_INCLUDE_DIRS AND JSONCPP_LIBRARIES )
        set( JSONCPP_FOUND true )
    endif( JSONCPP_INCLUDE_DIRS AND JSONCPP_LIBRARIES )

    if( JSONCPP_FOUND )
        if( NOT JSONCPP_FIND_QUIETLY )
            message( STATUS "Found JsonCpp (Required is at least version \"${JSONCPP_VERSION}\", found version \"${JSONCPP_FIND_VERSION}\")" )
            message( STATUS " - Includes: ${JSONCPP_INCLUDE_DIRS}" )
            message( STATUS " - Libraries: ${JSONCPP_LIBRARIES}" )
        endif( NOT JSONCPP_FIND_QUIETLY )
    else( JSONCPP_FOUND )
        if( JSONCPP_FIND_REQUIRED )
            message( FATAL_ERROR "Could not find JsonCpp" )
        endif( JSONCPP_FIND_REQUIRED )
    endif( JSONCPP_FOUND )

    mark_as_advanced( JSONCPP_INCLUDE_DIRS JSONCPP_LIBRARIES )

endif( JSONCPP_LIBRARIES AND JSONCPP_INCLUDE_DIRS )
