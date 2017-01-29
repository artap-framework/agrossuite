PROJECT(${POLY2TRI_LIBRARY})

SET(SOURCES common/shapes.cc 
    sweep/advancing_front.cc 
    sweep/cdt.cc 
    sweep/sweep.cc 
    sweep/sweep_context.cc
  )

IF (AGROS_BUILD_STATIC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME})
