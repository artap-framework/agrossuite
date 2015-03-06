FIND_PACKAGE(PackageHandleStandardArgs)

FIND_PATH(MKL_ROOT_DIR
  include/mkl_cblas.h
  PATHS
    $ENV{MKLDIR}
    /opt/intel/mkl/
    /opt/intel/cmkl/
    /opt/intel/Compiler/*/*/mkl
)

FIND_PATH(MKL_INCLUDE_DIRS
  mkl_cblas.h
  PATHS
    ${MKL_ROOT_DIR}/include
    /opt/intel/*/mkl/include
    ${INCLUDE_INSTALL_DIR}
)

IF(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  FIND_LIBRARY(MKL_LIBRARIES
    mkl_core
    PATHS
      ${MKL_ROOT_DIR}/lib/em64t
      ${MKL_ROOT_DIR}/lib/intel64
      $ENV{MKLLIB}
      $ENV{LD_LIBRARY_PATH}
  )
  IF(MKL_LIBRARIES)
    SET(MKL_LIBRARIES ${MKL_LIBRARIES} mkl_intel_lp64 mkl_sequential pthread)
  ENDIF()
ELSE()
  FIND_LIBRARY(MKL_LIBRARIES
    mkl_core
    PATHS
      ${MKL_ROOT_DIR}/lib/32
      ${MKL_ROOT_DIR}/lib/ia32
      $ENV{MKLLIB}
      $ENV{LD_LIBRARY_PATH}
  )
  IF(MKL_LIBRARIES)
    SET(MKL_LIBRARIES ${MKL_LIBRARIES} mkl_intel mkl_sequential pthread)
  ENDIF()
ENDIF()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(MKL DEFAULT_MSG MKL_INCLUDE_DIRS MKL_LIBRARIES)

MARK_AS_ADVANCED(MKL_LIBRARIES MKL_INCLUDE_DIRS)

