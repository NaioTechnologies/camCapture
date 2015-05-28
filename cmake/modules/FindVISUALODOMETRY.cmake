#-------------------------------------------------------------------------------
#
#   LIBVISUALODOMETRY_FOUND           - system has libVISUALODOMETRY
#   LIBVISUALODOMETRY_INCLUDE_DIRS    - the libVISUALODOMETRY include directory
#   LIBVISUALODOMETRY_LIBRARIES       - Link these to use libVISUALODOMETRY
#   LIBVISUALODOMETRY_DEFINITIONS     - Compiler switches required for using libVISUALODOMETRY
#   LIBVISUALODOMETRY_VERSION         - libVISUALODOMETRY version
#
#-------------------------------------------------------------------------------

if( VISUALODOMETRY_LIBRARIES AND VISUALODOMETRY_INCLUDE_DIRS )
    set( VISUALODOMETRY_FOUND true )
else( VISUALODOMETRY_LIBRARIES AND VISUALODOMETRY_INCLUDE_DIRS )

    # Looking for include folders
    find_path( VISUALODOMETRY_INCLUDE_DIR
        NAMES
            BuildVersion.hpp
        PATHS
            $ENV{LIB_DIR}/include
            /usr/include
            /usr/local/include
        PATH_SUFFIXES
            visualOdometry
    )
    SET( VISUALODOMETRY_INCLUDE_DIRS ${VISUALODOMETRY_INCLUDE_DIR} )

    # Looking for library folders
    find_library( VISUALODOMETRY_LIBRARY
        NAMES
            visualOdometry
        HINTS
            $ENV{LIB_DIR}/lib
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
    )
    SET( VISUALODOMETRY_LIBRARIES ${VISUALODOMETRY_LIBRARY} )

    macro( check_version )
        file( READ "${VISUALODOMETRY_INCLUDE_DIR}/BuildVersion.hpp" BUILD_VERSION_HEADER )

        # Major version
        string( REGEX MATCH "define[ \t]+LIBRARY_MAJOR_VERSION[ \t]+([0-9]+)+"
                VISUALODOMETRY_MAJOR_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
        set( VISUALODOMETRY_MAJOR_VERSION "${CMAKE_MATCH_1}" )

        # Minor version
        string( REGEX MATCH "define[ \t]+LIBRARY_MINOR_VERSION[ \t]+([0-9]+)+"
                VISUALODOMETRY_MINOR_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
        set( VISUALODOMETRY_MINOR_VERSION "${CMAKE_MATCH_1}" )

        # Patch version
        string( REGEX MATCH "define[ \t]+LIBRARY_PATCH_VERSION[ \t]+([0-9]+)+"
                VISUALODOMETRY_PATCH_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
        set( VISUALODOMETRY_PATCH_VERSION "${CMAKE_MATCH_1}" )

        set( VISUALODOMETRY_VERSION ${VISUALODOMETRY_MAJOR_VERSION}.${VISUALODOMETRY_MINOR_VERSION}.${VISUALODOMETRY_PATCH_VERSION} )

        if( ${VISUALODOMETRY_VERSION} VERSION_LESS ${VISUALODOMETRY_FIND_VERSION} )
            set( VISUALODOMETRY_VERSION_OK FALSE )
        else( ${VISUALODOMETRY_VERSION} VERSION_LESS ${VISUALODOMETRY_FIND_VERSION} )
            set( VISUALODOMETRY_VERSION_OK TRUE )
        endif( ${VISUALODOMETRY_VERSION} VERSION_LESS ${VISUALODOMETRY_FIND_VERSION} )

        if( NOT VISUALODOMETRY_VERSION_OK )
            message( STATUS "Robbie version \"${VISUALODOMETRY_VERSION}\" found in ${VISUALODOMETRY_INCLUDE_DIR}, "
                            "but at least version \"${VISUALODOMETRY_FIND_VERSION}\" is required" )
        endif( NOT VISUALODOMETRY_VERSION_OK )
    endmacro( check_version )

    check_version()

    if( VISUALODOMETRY_INCLUDE_DIRS AND VISUALODOMETRY_LIBRARIES AND VISUALODOMETRY_VERSION_OK )
        set( VISUALODOMETRY_FOUND true )
    endif( VISUALODOMETRY_INCLUDE_DIRS AND VISUALODOMETRY_LIBRARIES AND VISUALODOMETRY_VERSION_OK )

    if( VISUALODOMETRY_FOUND )
        if( NOT VISUALODOMETRY_FIND_QUIETLY )
            message( STATUS "Found Robbie (Required is at least version \"${VISUALODOMETRY_VERSION}\", found version \"${VISUALODOMETRY_FIND_VERSION}\")" )
            message( STATUS " - Includes: ${VISUALODOMETRY_INCLUDE_DIRS}" )
            message( STATUS " - Libraries: ${VISUALODOMETRY_LIBRARIES}" )
        endif( NOT VISUALODOMETRY_FIND_QUIETLY )
    else( VISUALODOMETRY_FOUND )
        if( VISUALODOMETRY_FIND_REQUIRED )
            message( FATAL_ERROR "Could not find VisualOdometry" )
        endif( VISUALODOMETRY_FIND_REQUIRED )
    endif( VISUALODOMETRY_FOUND )

    mark_as_advanced( VISUALODOMETRY_INCLUDE_DIRS VISUALODOMETRY_LIBRARIES )

endif( VISUALODOMETRY_LIBRARIES AND VISUALODOMETRY_INCLUDE_DIRS )
