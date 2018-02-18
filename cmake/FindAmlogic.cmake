find_path(AMLOGIC_INCLUDE_DIR
  NAMES codec.h
  DOC "Amlogic include directory"
  PATHS /usr/local/include/amcodec /usr/include/amcodec /usr/include/)
mark_as_advanced(AMLOGIC_INCLUDE_DIR)

find_library(AMCODEC_LIBRARY
  NAMES libamcodec.so
  DOC "Path to Amlogic Video Codec Library"
  PATHS /usr/lib/aml_libs /usr/local/lib /usr/lib)
mark_as_advanced(AMCODEC_LIBRARY)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Amlogic DEFAULT_MSG AMLOGIC_INCLUDE_DIR AMCODEC_LIBRARY)

set(AMLOGIC_LIBRARIES ${AMCODEC_LIBRARY})
set(AMLOGIC_INCLUDE_DIRS ${AMLOGIC_INCLUDE_DIR})
