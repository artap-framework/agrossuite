PROJECT(${MATIO_LIBRARY})

find_package(ZLIB REQUIRED)

add_custom_target(MATIO_CONFIG ALL DEPENDS matioConfig.h)

IF(MSVC)
  ADD_CUSTOM_COMMAND(
        SOURCE    matioConfigWin.h
        COMMAND   copy ARGS matioConfigWin.h matioConfig.h OUTPUT matioConfig.h
        TARGET    MATIO_CONFIG
        OUTPUTS   matioConfig.h)
ELSE()
  ADD_CUSTOM_COMMAND(
        SOURCE    matioConfigLinux.h
        COMMAND   cp ARGS matioConfigLinux.h matioConfig.h OUTPUT matioConfig.h
        TARGET    MATIO_CONFIG
        OUTPUTS   matioConfig.h)
ENDIF()
ADD_CUSTOM_COMMAND(
      SOURCE    MATIO_CONFIG
      TARGET    MATIO_CONFIG
      DEPENDS   matioConfig.h)

SET(SOURCES endian.c
    inflate.c
    io.c
    mat4.c
    mat5.c
    mat73.c
    mat.c
    matvar_cell.c
    matvar_struct.c
    read_data.c
    snprintf.c
    matioConfig.h
)

IF (AGROS_BUILD_STATIC OR MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ZLIB_LIBRARIES})
