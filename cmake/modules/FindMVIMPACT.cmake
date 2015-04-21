#-------------------------------------------------------------------------------
#
#   LIBMVIMPACT Module finder.
#
#   LIBMVIMPACT_FOUND           - system has libvitals
#   LIBMVIMPACT_INCLUDE_DIRS    - the libvitals include directory
#   LIBMVIMPACT_LIBRARIES       - Link these to use libvitals
#   LIBMVIMPACT_DEFINITIONS     - Compiler switches required for using libvitals
#   LIBMVIMPACT_VERSION         - libvitals version
#
#-------------------------------------------------------------------------------

if( MVIMPACT_LIBRARIES AND MVIMPACT_INCLUDE_DIRS )
    set( MVIMPACT_FOUND true )
else( MVIMPACT_LIBRARIES AND MVIMPACT_INCLUDE_DIRS )

    # Looking for include folders
    find_path( MVIMPACT_INCLUDE_DIR
        NAMES
            mvIMPACT_CPP/mvIMPACT_acquire.h
        PATHS
            /usr/include
            /usr/local/include
            /opt
        PATH_SUFFIXES
            mvIMPACT_acquire
    )

    SET( MVIMPACT_INCLUDE_DIRS ${MVIMPACT_INCLUDE_DIR} )

    # Looking for library folders
    find_library( MVBLUEFOX_LIBRARY
        NAMES
            libmvBlueFOX.so
		HINTS
            /opt/mvIMPACT_acquire/lib/x86_64
            /opt/mvIMPACT_acquire/lib
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
    )

    # Looking for library folders
    find_library( MVDEVICEMANAGER_LIBRARY
        NAMES
            libmvDeviceManager.so
		HINTS
            /opt/mvIMPACT_acquire/lib/x86_64
            /opt/mvIMPACT_acquire/lib
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
    )

    SET( MVIMPACT_LIBRARIES ${MVBLUEFOX_LIBRARY} ${MVDEVICEMANAGER_LIBRARY} )

    if( MVIMPACT_INCLUDE_DIRS AND MVIMPACT_LIBRARIES )
        set( MVIMPACT_FOUND true )
    endif( MVIMPACT_INCLUDE_DIRS AND MVIMPACT_LIBRARIES )

    if( MVIMPACT_FOUND )
        if( NOT MVIMPACT_FIND_QUIETLY )
            message( STATUS "Found mvIMPACT" )
            message( STATUS " - Includes: ${MVIMPACT_INCLUDE_DIRS}" )
            message( STATUS " - Libraries: ${MVIMPACT_LIBRARIES}" )
        endif( NOT MVIMPACT_FIND_QUIETLY )
    else( MVIMPACT_FOUND )
        if( MVIMPACT_FIND_REQUIRED )
            message( FATAL_ERROR "Could not find mvIMPACT" )
        endif( MVIMPACT_FIND_REQUIRED )
    endif( MVIMPACT_FOUND )

    mark_as_advanced( MVIMPACT_INCLUDE_DIRS MVIMPACT_LIBRARIES )

endif( MVIMPACT_LIBRARIES AND MVIMPACT_INCLUDE_DIRS )
