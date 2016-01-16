# - Try to find LIBUUID 
# Find LIBUUID headers, libraries and the answer to all questions.
#
#  LIBUUID_FOUND               True if libuuid got found
#  LIBUUID_INCLUDE_DIRS        Location of libuuid headers 
#  LIBUUID_LIBRARIES           List of libraries to use libuuid 
#
# Copyright (c) 2008 Bjoern Ricks <bjoern.ricks@googlemail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

INCLUDE( FindPkgConfig )

IF ( LibUuid_FIND_REQUIRED )
	SET( _pkgconfig_REQUIRED "REQUIRED" )
ELSE( LibUuid_FIND_REQUIRED )
	SET( _pkgconfig_REQUIRED "" )	
ENDIF ( LibUuid_FIND_REQUIRED )

IF ( LIBUUID_MIN_VERSION )
	PKG_SEARCH_MODULE( LIBUUID ${_pkgconfig_REQUIRED} uuid>=${LIBUUID_MIN_VERSION} )
ELSE ( LIBUUID_MIN_VERSION )
	PKG_SEARCH_MODULE( LIBUUID ${_pkgconfig_REQUIRED} uuid )
ENDIF ( LIBUUID_MIN_VERSION )


IF( NOT LIBUUID_FOUND AND NOT PKG_CONFIG_FOUND )
	FIND_PATH( LIBUUID_INCLUDE_DIRS uuid/uuid.h )
	FIND_LIBRARY( LIBUUID_LIBRARIES uuid)

	# Report results
	IF ( LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS )	
		SET( LIBUUID_FOUND 1 )
		IF ( NOT LIBUUID_FIND_QUIETLY )
			MESSAGE( STATUS "Found libuuid: ${LIBUUID_LIBRARIES}" )
		ENDIF ( NOT LIBUUID_FIND_QUIETLY )
	ELSE ( LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS )	
		IF ( LIBUUID_FIND_REQUIRED )
			MESSAGE( SEND_ERROR "Could NOT find libuuid" )
		ELSE ( LIBUUID_FIND_REQUIRED )
			IF ( NOT LIBUUID_FIND_QUIETLY )
				MESSAGE( STATUS "Could NOT find libuuid" )	
			ENDIF ( NOT LIBUUID_FIND_QUIETLY )
		ENDIF ( LIBUUID_FIND_REQUIRED )
	ENDIF ( LIBUUID_LIBRARIES AND LIBUUID_INCLUDE_DIRS )
ENDIF( NOT LIBUUID_FOUND AND NOT PKG_CONFIG_FOUND )

MARK_AS_ADVANCED( LIBUUID_LIBRARIES LIBUUID_INCLUDE_DIRS )
