PROJECT(${MATIO_LIBRARY})

set_property(SOURCE matioConfig.h PROPERTY SKIP_AUTOMOC ON)
add_custom_target(MATIO_CONFIG ALL DEPENDS matioConfig.h)

IF(MSVC)
  configure_file(matioConfigWin.h matioConfig.h COPYONLY)
ELSE()
  configure_file(matioConfigLinux.h matioConfig.h COPYONLY)
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

message("MATIO ZLIB: ${ZLIB_LIBRARY}")

IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ZLIB_LIBRARY})
