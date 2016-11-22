PROJECT(${CMAES_LIBRARY})

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/boost-1.56.0/include/")

#SET(HEADERS)
SET(SOURCES src/acovarianceupdate.cc
        src/cmaparameters.cc
        src/cmastopcriteria.cc
        src/covarianceupdate.cc
        src/esostrategy.cc
        src/pwq_bound_strategy.cc
        src/vdcmaupdate.cc
        src/bipopcmastrategy.cc
        src/cmasolutions.cc
        src/cmastrategy.cc
        src/errstats.cc
        src/ipopcmastrategy.cc
        src/surrogatestrategy.cc)

ADD_LIBRARY(${PROJECT_NAME} ${HEADERS} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
