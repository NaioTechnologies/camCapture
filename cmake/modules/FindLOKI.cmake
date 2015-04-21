#-------------------------------------------------------------------------------
#   LibLOKI Module finder.
#
#   LOKI_FOUND - system has LOKI
#   LOKI_INCLUDE_DIRS - the LOKI include directory
#   LOKI_LIBRARIES - Link these to use LOKI
#-------------------------------------------------------------------------------

IF( LOKI_LIBRARIES AND LOKI_INCLUDE_DIRS )
    SET( LOKI_FOUND TRUE )
ELSE( LOKI_LIBRARIES AND LOKI_INCLUDE_DIRS )

    SET( CUSTOM_LIBRARY_PATH $ENV{LIB_DIR} )
    FIND_PATH( LOKI_INCLUDE_DIR
        NAMES
            SmallObj.h
        PATHS
            $ENV{LIB_DIR}/include
            /usr/include
            /usr/local/include
        PATH_SUFFIXES
            loki
    )

    FIND_LIBRARY( LOKI_LIBRARY
        NAMES
            loki
        HINTS
            $ENV{LIB_DIR}
        PATH
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
    )


    SET( LOKI_INCLUDE_DIRS  ${LOKI_INCLUDE_DIR} )
    SET( LOKI_LIBRARIES     ${LOKI_LIBRARY} )

    if( LOKI_INCLUDE_DIRS AND LOKI_LIBRARIES )
        set( LOKI_FOUND TRUE )
    endif( LOKI_INCLUDE_DIRS AND LOKI_LIBRARIES )

    if( LOKI_FOUND )
        if( NOT LOKI_FIND_QUIETLY )
            message( STATUS "Found libloki:" )
            message( STATUS " - Includes: ${LOKI_INCLUDE_DIRS}" )
            message( STATUS " - Libraries: ${LOKI_LIBRARIES}" )
        endif( NOT LOKI_FIND_QUIETLY )
    else( LOKI_FOUND )
        if( LOKI_FIND_REQUIRED )
            message( FATAL_ERROR "Could not find LOKI" )
        endif( LOKI_FIND_REQUIRED )
    endif( LOKI_FOUND )

    mark_as_advanced( LOKI_INCLUDE_DIRS LOKI_LIBRARIES )

endif( LOKI_LIBRARIES AND LOKI_INCLUDE_DIRS )
