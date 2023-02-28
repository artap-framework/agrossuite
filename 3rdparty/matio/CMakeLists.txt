PROJECT(${MATIO_LIBRARY})

set_property(SOURCE matioConfig.h PROPERTY SKIP_AUTOMOC ON)
add_custom_target(MATIO_CONFIG ALL DEPENDS matioConfig.h)

IF(MSVC)
  ADD_CUSTOM_COMMAND(
        COMMAND   copy ARGS matioConfigWin.h matioConfig.h OUTPUT matioConfig.h
        DEPENDS    matioConfigWin.h
        OUTPUTS   matioConfig.h)
ELSE()
  ADD_CUSTOM_COMMAND(
        COMMAND   cp ARGS matioConfigLinux.h matioConfig.h OUTPUT matioConfig.h
        DEPENDS    matioConfigLinux.h
        OUTPUTS   matioConfig.h)
ENDIF()

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

IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ZLIB_LIBRARY})
