find_path(AMLOGIC_INCLUDE_DIR
  NAMES codec.h
  DOC "Amlogic include directory"
  PATHS /usr/local/include/amcodec /usr/include/amcodec)
mark_as_advanced(AMLOGIC_INCLUDE_DIR)

find_library(AMAVUTILS_LIBRARY
  NAMES libamavutils.so
  DOC "Path to Amlogic Audio Video Utils Library"
  PATHS /usr/lib/aml_libs /usr/local/lib /usr/lib)
mark_as_advanced(AMAVUTILS_LIBRARY)

find_library(AMADEC_LIBRARY
  NAMES libamadec.so
  DOC "Path to Amlogic Audio Decoder Library"
  PATHS /usr/lib/aml_libs /usr/local/lib /usr/lib)
mark_as_advanced(AMADEC_LIBRARY)

find_library(AMCODEC_LIBRARY
  NAMES libamcodec.so
  DOC "Path to Amlogic Video Codec Library"
  PATHS /usr/lib/aml_libs /usr/local/lib /usr/lib)
mark_as_advanced(AMCODEC_LIBRARY)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Amlogic DEFAULT_MSG AMLOGIC_INCLUDE_DIR AMCODEC_LIBRARY AMADEC_LIBRARY AMAVUTILS_LIBRARY)

set(AMLOGIC_LIBRARIES ${AMCODEC_LIBRARY} ${AMADEC_LIBRARY} ${AMAVUTILS_LIBRARY})
set(AMLOGIC_INCLUDE_DIRS ${AMLOGIC_INCLUDE_DIR})
