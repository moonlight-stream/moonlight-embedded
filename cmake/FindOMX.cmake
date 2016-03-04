find_path(BCM_INCLUDE_DIR
  NAMES bcm_host.h
  DOC "BCM include directory"
  PATHS /opt/vc/include)
mark_as_advanced(BCM_INCLUDE_DIR)

find_path(IL_INCLUDE_DIR
  NAMES ilclient.h
  DOC "ILClient include directory"
  PATHS /opt/vc/src/hello_pi/libs/ilclient)
mark_as_advanced(IL_INCLUDE_DIR)

find_library(BCM_HOST_LIBRARY
  NAMES libbcm_host.so
  DOC "Path to BCM Host Library"
  PATHS /opt/vc/lib)
mark_as_advanced(BCM_HOST_LIBRARY)

find_library(VCHIQ_LIBRARY
  NAMES libvchiq_arm.so
  DOC "Path to VCHIQ Library"
  PATHS /opt/vc/lib)
mark_as_advanced(VCHIQ_LIBRARY)

find_library(OPENMAXIL_LIBRARY
  NAMES libopenmaxil.so
  DOC "Path to OpenMAX IL Library"
  PATHS /opt/vc/lib)
mark_as_advanced(OPENMAXIL_LIBRARY)

find_library(VCOS_LIBRARY
  NAMES libvcos.so
  DOC "Path to VCOS Library"
  PATHS /opt/vc/lib)
mark_as_advanced(VCOS_LIBRARY)

find_library(ILCLIENT_LIBRARY
  NAMES libilclient.a
  DOC "Path to IL Client Library"
  PATHS /opt/vc/src/hello_pi/libs/ilclient)
mark_as_advanced(ILCLIENT_LIBRARY)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OMX DEFAULT_MSG IL_INCLUDE_DIR BCM_INCLUDE_DIR BCM_HOST_LIBRARY ILCLIENT_LIBRARY VCOS_LIBRARY VCHIQ_LIBRARY OPENMAXIL_LIBRARY)

set(OMX_LIBRARIES ${BCM_HOST_LIBRARY} ${ILCLIENT_LIBRARY} ${VCOS_LIBRARY} ${VCHIQ_LIBRARY} ${OPENMAXIL_LIBRARY})
set(OMX_INCLUDE_DIRS ${BCM_INCLUDE_DIR} ${BCM_INCLUDE_DIR}/IL ${IL_INCLUDE_DIR} ${BCM_INCLUDE_DIR}/interface/vcos/pthreads ${BCM_INCLUDE_DIR}/interface/vmcs_host/linux)
set(OMX_DEFINITIONS USE_VCHIQ_ARM HAVE_LIBOPENMAX=2 OMX OMX_SKIP64BIT USE_EXTERNAL_OMX HAVE_LIBBCM_HOST USE_EXTERNAL_LIBBCM_HOST)
