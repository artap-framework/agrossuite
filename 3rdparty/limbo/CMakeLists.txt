PROJECT(${LIMBO_LIBRARY})

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/boost-1.56.0/include/")

#SET(HEADERS)
SET(SOURCES ehvi/ehvi_calculations.cc
    ehvi/ehvi_hvol.cc
    ehvi/ehvi_montecarlo.cc
    ehvi/ehvi_multi.cc
    ehvi/ehvi_sliceupdate.cc
    ehvi/helper.cc
    hv/hypervol.c)

ADD_LIBRARY(${PROJECT_NAME} ${HEADERS} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
