PROJECT(${LIMBO_LIBRARY})

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/boost-1.62.0/include/")

#SET(HEADERS)
SET(SOURCES ehvi/ehvi_calculations.cc
    ehvi/ehvi_hvol.cc
    ehvi/ehvi_montecarlo.cc
    ehvi/ehvi_multi.cc
    ehvi/ehvi_sliceupdate.cc
    ehvi/helper.cc
    hv/hypervol.c)

IF (AGROS_BUILD_STATIC OR MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME})
