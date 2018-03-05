find_path(DRM_INCLUDE_DIR
  NAMES drm.h
  DOC "libdrm include directory"
  PATHS /usr/local/include/libdrm /usr/include/libdrm /usr/include)
mark_as_advanced(DRM_INCLUDE_DIR)

find_path(DRM_LIBRARY
  NAMES libdrm.so
  DOC "Path to libdrm Library"
  PATHS /usr/local/lib /usr/lib /usr/lib/aarch64-linux-gnu /usr/lib/arm-linux-gnueabihf)
mark_as_advanced(DRM_INCLUDE_DIR)

find_path(ROCKCHIP_INCLUDE_DIR
  NAMES rk_mpi.h
  DOC "Rockchip include directory"
  PATHS /usr/local/include/rockchip /usr/include/rockchip /usr/include)
mark_as_advanced(ROCKCHIP_INCLUDE_DIR)

find_library(ROCKCHIP_LIBRARY
  NAMES librockchip_mpp.so
  DOC "Path to Rockchip Media Process Platform Library"
  PATHS /usr/local/lib /usr/lib /usr/lib/aarch64-linux-gnu /usr/lib/arm-linux-gnueabihf)
mark_as_advanced(ROCKCHIP_LIBRARY)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Rockchip DEFAULT_MSG ROCKCHIP_INCLUDE_DIR ROCKCHIP_LIBRARY)

set(ROCKCHIP_INCLUDE_DIRS ${ROCKCHIP_INCLUDE_DIR} ${DRM_INCLUDE_DIR})
set(ROCKCHIP_LIBRARIES ${ROCKCHIP_LIBRARY} ${DRM_LIBRARY})
