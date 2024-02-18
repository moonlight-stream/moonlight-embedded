# CMake - Cross Platform Makefile Generator
# Copyright 2000-2024 Kitware, Inc. and Contributors
# All rights reserved.
#
# Distributed under the OSI-approved BSD 3-Clause License. See
# https://cmake.org/licensing for details.
#
#[=======================================================================[.rst:
FindLibUUID
------------

Find LibUUID include directory and library.

Imported Targets
^^^^^^^^^^^^^^^^

An :ref:`imported target <Imported targets>` named
``LibUUID::LibUUID`` is provided if LibUUID has been found.

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``LibUUID_FOUND``
  True if LibUUID was found, false otherwise.
``LibUUID_INCLUDE_DIRS``
  Include directories needed to include LibUUID headers.
``LibUUID_LIBRARIES``
  Libraries needed to link to LibUUID.

Cache Variables
^^^^^^^^^^^^^^^

This module uses the following cache variables:

``LibUUID_LIBRARY``
  The location of the LibUUID library file.
``LibUUID_INCLUDE_DIR``
  The location of the LibUUID include directory containing ``uuid/uuid.h``.

The cache variables should not be used by project code.
They may be set by end users to point at LibUUID components.
#]=======================================================================]

#-----------------------------------------------------------------------------
find_library(LibUUID_LIBRARY
  NAMES uuid
  )
mark_as_advanced(LibUUID_LIBRARY)

find_path(LibUUID_INCLUDE_DIR
  NAMES uuid/uuid.h
  )
mark_as_advanced(LibUUID_INCLUDE_DIR)

#-----------------------------------------------------------------------------
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibUUID
  FOUND_VAR LibUUID_FOUND
  REQUIRED_VARS LibUUID_LIBRARY LibUUID_INCLUDE_DIR
  )
set(LIBUUID_FOUND ${LibUUID_FOUND})

#-----------------------------------------------------------------------------
# Provide documented result variables and targets.
if(LibUUID_FOUND)
  set(LibUUID_INCLUDE_DIRS ${LibUUID_INCLUDE_DIR})
  set(LibUUID_LIBRARIES ${LibUUID_LIBRARY})
  if(NOT TARGET LibUUID::LibUUID)
    add_library(LibUUID::LibUUID UNKNOWN IMPORTED)
    set_target_properties(LibUUID::LibUUID PROPERTIES
      IMPORTED_LOCATION "${LibUUID_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LibUUID_INCLUDE_DIRS}"
      )
  endif()
endif()
