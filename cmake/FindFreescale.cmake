find_path(FREESCALE_INCLUDE_DIR
  NAMES vpu_lib.h
  DOC "Freescale include directory"
  PATHS /opt/fsl/include)
mark_as_advanced(BROADCOM_INCLUDE_DIR)

find_path(KERNEL_INCLUDE_DIR
  NAMES linux/mxc_v4l2.h
  DOC "Kernel include directory"
  PATHS /lib/modules/${CMAKE_SYSTEM_VERSION}/build/include)
mark_as_advanced(KERNEL_INCLUDE_DIR)

find_library(VPU_LIBRARY
  NAMES libvpu.so
  DOC "Path to Freescale VPU Library"
  PATHS /opt/fsl/lib)
mark_as_advanced(VCOS_LIBRARY)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Freescale DEFAULT_MSG FREESCALE_INCLUDE_DIR KERNEL_INCLUDE_DIR VPU_LIBRARY)

set(FREESCALE_LIBRARIES ${VPU_LIBRARY})
set(FREESCALE_INCLUDE_DIRS ${FREESCALE_INCLUDE_DIR} ${KERNEL_INCLUDE_DIR})
