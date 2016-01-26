PROJECT(${AGROS_UTIL})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAGROS_UTIL_DLL")

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ENDIF()

SET(SOURCES util.cpp
    util/checkversion.cpp 
    util/point.cpp 
    util/system_utils.cpp 
    util/spline.cpp
    util/table.cpp
    gui/about.cpp 
    gui/filebrowser.cpp 
    gui/lineeditdouble.cpp 
    gui/systemoutput.cpp 
    gui/textedit.cpp)

SET(HEADERS util.h
    sparse_io.h
    util/checkversion.h 
    util/point.h 
    util/system_utils.h 
    util/spline.h
    util/table.h
    gui/about.h 
    gui/filebrowser.h 
    gui/lineeditdouble.h 
    gui/systemoutput.h 
    gui/textedit.h)
    
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network OpenGL Xml XmlPatterns WebKit WebKitWidgets OpenGL)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# solver_export_matlab
SET(SOURCES_STAT solver_export_matlab.cpp)
SET(HEADERS_STAT sparse_io.h)

message(${MATIO_LIBRARY})

ADD_EXECUTABLE(solver_export_matlab ${SOURCES_STAT} ${HEADERS_STAT})
TARGET_LINK_LIBRARIES(solver_export_matlab ${MATIO_LIBRARY})
SET_TARGET_PROPERTIES(solver_export_matlab PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/libs)
INSTALL(TARGETS solver_export_matlab DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
