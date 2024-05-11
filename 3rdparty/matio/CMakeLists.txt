PROJECT(${MATIO_LIBRARY})

set_property(SOURCE matioConfig.h PROPERTY SKIP_AUTOMOC ON)
add_custom_target(MATIO_CONFIG ALL DEPENDS matioConfig.h)

IF(MSVC)
  ADD_CUSTOM_COMMAND(
        COMMAND  ${CMAKE_COMMAND} -E copy ${CMAKE_AGROS_DIRECTORY}3rdparty/matio/matioConfigWin.h ${CMAKE_AGROS_DIRECTORY}3rdparty/matio/matioConfig.h 
	OUTPUT matioConfig.h DEPENDS  ${CMAKE_AGROS_DIRECTORY}3rdparty/matio/matioConfigWin.h})
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

message("MATIO ZLIB: ${ZLIB_LIBRARY}")

IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ZLIB_LIBRARY})
