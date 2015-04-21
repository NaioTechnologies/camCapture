#-------------------------------------------------------------------------------
#
#   Robbie Module finder.
#
#   LIBROBBIE_FOUND           - system has librobbie
#   LIBROBBIE_INCLUDE_DIRS    - the librobbie include directory
#   LIBROBBIE_LIBRARIES       - Link these to use librobbie
#   LIBROBBIE_DEFINITIONS     - Compiler switches required for using librobbie
#   LIBROBBIE_VERSION         - librobbie version
#
#-------------------------------------------------------------------------------

if( ROBBIE_LIBRARIES AND ROBBIE_INCLUDE_DIRS )
    set( ROBBIE_FOUND true )
else( ROBBIE_LIBRARIES AND ROBBIE_INCLUDE_DIRS )

    # Looking for include folders
    find_path( ROBBIE_INCLUDE_DIR
        NAMES
            BuildVersion.h
        PATHS
            $ENV{LIB_DIR}/include
            /usr/include
            /usr/local/include
        PATH_SUFFIXES
            robbie
    )
    SET( ROBBIE_INCLUDE_DIRS ${ROBBIE_INCLUDE_DIR} )

    # Looking for library folders
    find_library( ROBBIE_LIBRARY
        NAMES
            robbie
        HINTS
            $ENV{LIB_DIR}/lib
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
    )
    SET( ROBBIE_LIBRARIES ${ROBBIE_LIBRARY} )

    macro( check_version )
        file( READ "${ROBBIE_INCLUDE_DIR}/BuildVersion.h" BUILD_VERSION_HEADER )

        # Major version
        string( REGEX MATCH "define[ \t]+LIBRARY_MAJOR_VERSION[ \t]+([0-9]+)+"
                ROBBIE_MAJOR_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
        set( ROBBIE_MAJOR_VERSION "${CMAKE_MATCH_1}" )

        # Minor version
        string( REGEX MATCH "define[ \t]+LIBRARY_MINOR_VERSION[ \t]+([0-9]+)+"
                ROBBIE_MINOR_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
        set( ROBBIE_MINOR_VERSION "${CMAKE_MATCH_1}" )

        # Patch version
        string( REGEX MATCH "define[ \t]+LIBRARY_PATCH_VERSION[ \t]+([0-9]+)+"
                ROBBIE_PATCH_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
        set( ROBBIE_PATCH_VERSION "${CMAKE_MATCH_1}" )

        set( ROBBIE_VERSION ${ROBBIE_MAJOR_VERSION}.${ROBBIE_MINOR_VERSION}.${ROBBIE_PATCH_VERSION} )

        if( ${ROBBIE_VERSION} VERSION_LESS ${ROBBIE_FIND_VERSION} )
            set( ROBBIE_VERSION_OK FALSE )
        else( ${ROBBIE_VERSION} VERSION_LESS ${ROBBIE_FIND_VERSION} )
            set( ROBBIE_VERSION_OK TRUE )
        endif( ${ROBBIE_VERSION} VERSION_LESS ${ROBBIE_FIND_VERSION} )

        if( NOT ROBBIE_VERSION_OK )
            message( STATUS "Robbie version \"${ROBBIE_VERSION}\" found in ${ROBBIE_INCLUDE_DIR}, "
                            "but at least version \"${ROBBIE_FIND_VERSION}\" is required" )
        endif( NOT ROBBIE_VERSION_OK )
    endmacro( check_version )

    check_version()

    if( ROBBIE_INCLUDE_DIRS AND ROBBIE_LIBRARIES AND ROBBIE_VERSION_OK )
        set( ROBBIE_FOUND true )
    endif( ROBBIE_INCLUDE_DIRS AND ROBBIE_LIBRARIES AND ROBBIE_VERSION_OK )

    if( ROBBIE_FOUND )
        if( NOT ROBBIE_FIND_QUIETLY )
            message( STATUS "Found Robbie (Required is at least version \"${ROBBIE_VERSION}\", found version \"${ROBBIE_FIND_VERSION}\")" )
            message( STATUS " - Includes: ${ROBBIE_INCLUDE_DIRS}" )
            message( STATUS " - Libraries: ${ROBBIE_LIBRARIES}" )
        endif( NOT ROBBIE_FIND_QUIETLY )
    else( ROBBIE_FOUND )
        if( ROBBIE_FIND_REQUIRED )
            message( FATAL_ERROR "Could not find Robbie" )
        endif( ROBBIE_FIND_REQUIRED )
    endif( ROBBIE_FOUND )

    mark_as_advanced( ROBBIE_INCLUDE_DIRS ROBBIE_LIBRARIES )

endif( ROBBIE_LIBRARIES AND ROBBIE_INCLUDE_DIRS )
