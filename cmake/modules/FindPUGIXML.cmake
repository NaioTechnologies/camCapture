#-------------------------------------------------------------------------------
#
#	PugiXML Module finder.
#
#	PUGIXML_FOUND           - system has libpugixml
#	PUGIXML_INCLUDE_DIRS    - the libpugixml include directory
#	PUGIXML_LIBRARIES       - Link these to use libpugixml
#   PUGIXML_DEFINITIONS     - Compiler switches required for using libpugixml
#   PUGIXML_VERSION         - libpugixml version
#
#-------------------------------------------------------------------------------

if( PUGIXML_LIBRARIES AND PUGIXML_INCLUDE_DIRS )
	set( PUGIXML_FOUND true )
else( PUGIXML_LIBRARIES AND PUGIXML_INCLUDE_DIRS )

    # Setting variables
	set( CUSTOM_LIBRARY_PATH $ENV{LIB_DIR} )

    # Looking for include folders
	find_path( PUGIXML_INCLUDE_DIR
		NAMES
			pugixml.hpp
		PATHS
			$ENV{LIB_DIR}/include
            /usr/include
            /usr/local/include
		PATH_SUFFIXES
	)
    SET( PUGIXML_INCLUDE_DIRS ${PUGIXML_INCLUDE_DIR} )

    # Looking for library folders
	find_library( PUGIXML_LIBRARY
		NAMES
			pugixml
		HINTS
			$ENV{LIB_DIR}
		PATH
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            /lib/i386-linux-gnu
	)
    SET( PUGIXML_LIBRARIES ${PUGIXML_LIBRARY} )

    # # Checking default library version
    # if( NOT PUGIXML_FIND_VERSION )
    #     if( NOT PUGIXML_FIND_VERSION_MAJOR )
    #         set( PUGIXML_FIND_VERSION_MAJOR 0 )
    #     endif( NOT PUGIXML_FIND_VERSION_MAJOR )
    #     if( NOT PUGIXML_FIND_VERSION_MINOR )
    #         set( PUGIXML_FIND_VERSION_MINOR 3 )
    #     endif( NOT PUGIXML_FIND_VERSION_MINOR )
    #     if( NOT PUGIXML_FIND_VERSION_PATCH )
    #         set( PUGIXML_FIND_VERSION_PATCH 0 )
    #     endif( NOT PUGIXML_FIND_VERSION_PATCH )
    #     set( PUGIXML_FIND_VERSION "${PUGIXML_FIND_VERSION_MAJOR}.${PUGIXML_FIND_VERSION_MINOR}.${PUGIXML_FIND_VERSION_PATCH}" )
    # endif( NOT PUGIXML_FIND_VERSION )

    # macro( check_version )
    #     file( READ "${PUGIXML_INCLUDE_DIR}/BuildVersion.h" BUILD_VERSION_HEADER )

    #     # Major version
    #     string( REGEX MATCH "define[ \t]+LIBRARY_MAJOR_VERSION[ \t]+[\"]([0-9]+)+[\"]" LIBRARY_MAJOR_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
    #     set( LIBRARY_MAJOR_VERSION "${CMAKE_MATCH_1}" )

    #     # Minor version
    #     string( REGEX MATCH "define[ \t]+LIBRARY_MINOR_VERSION[ \t]+[\"]([0-9]+)+[\"]" LIBRARY_MINOR_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
    #     set( LIBRARY_MINOR_VERSION "${CMAKE_MATCH_1}" )

    #     # Patch version
    #     string( REGEX MATCH "define[ \t]+LIBRARY_PATCH_VERSION[ \t]+[\"]([0-9]+)+[\"]" LIBRARY_PATCH_VERSION_MATCH "${BUILD_VERSION_HEADER}" )
    #     set( LIBRARY_PATCH_VERSION "${CMAKE_MATCH_1}" )

    #     set( VITALS_VERSION ${LIBRARY_MAJOR_VERSION}.${LIBRARY_MINOR_VERSION}.${LIBRARY_PATCH_VERSION} )

    #     if( ${VITALS_VERSION} VERSION_LESS ${PUGIXML_FIND_VERSION} )
    #         set( VITALS_VERSION_OK FALSE )
    #     else( ${VITALS_VERSION} VERSION_LESS ${PUGIXML_FIND_VERSION} )
    #         set( VITALS_VERSION_OK TRUE )
    #     endif( ${VITALS_VERSION} VERSION_LESS ${PUGIXML_FIND_VERSION} )

    #     if( NOT VITALS_VERSION_OK )
    #         message( STATUS "Vitals version \"${VITALS_VERSION}\" found in ${PUGIXML_INCLUDE_DIR}, "
    #                      "but at least version \"${PUGIXML_FIND_VERSION}\" is required" )
    #     endif( NOT VITALS_VERSION_OK )
    # endmacro( check_version )

    # check_version()

	if( PUGIXML_INCLUDE_DIRS AND PUGIXML_LIBRARIES )
		set( PUGIXML_FOUND true )
	endif( PUGIXML_INCLUDE_DIRS AND PUGIXML_LIBRARIES )

	if( PUGIXML_FOUND )
		if( NOT PUGIXML_FIND_QUIETLY )
            message( STATUS "Found PugiXML" )
            message( STATUS " - Includes: ${PUGIXML_INCLUDE_DIRS}" )
            message( STATUS " - Libraries: ${PUGIXML_LIBRARIES}" )
		endif( NOT PUGIXML_FIND_QUIETLY )
	else( PUGIXML_FOUND )
		if( PUGIXML_FIND_REQUIRED )
			message( FATAL_ERROR "Could not find PugiXML" )
		endif( PUGIXML_FIND_REQUIRED )
	endif( PUGIXML_FOUND )

	mark_as_advanced( PUGIXML_INCLUDE_DIRS PUGIXML_LIBRARIES )

endif( PUGIXML_LIBRARIES AND PUGIXML_INCLUDE_DIRS )
