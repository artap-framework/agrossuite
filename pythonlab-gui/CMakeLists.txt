PROJECT(${PYTHONLAB})

# Python
FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)

SET(SOURCES main.cpp
    pythonbrowser.cpp
    pythonconsole.cpp
    pythonhighlighter.cpp
    pythoncompleter.cpp
    pythoneditor.cpp
    pythonengine.cpp
    pythonlabwindow.cpp
    other/filebrowser.cpp
    other/other.cpp
    other/textedit.cpp
    other/util.cpp)

SET(HEADERS pythonbrowser.h
    pythonconsole.h
    pythonhighlighter.h
    pythoncompleter.h
    pythoneditor.h
    pythonengine.h
    pythonlabwindow.h
    other/filebrowser.h
    other/other.h
    other/textedit.h
    other/util.h)

# images
SET(RESOURCES ../resources_source/resources.qrc)
QT5_ADD_RESOURCES(RESOURCES_RCC ${RESOURCES})

IF(WIN32)
  SET(GUI WIN32)
  ADD_DEFINITIONS(-D_AFXDLL)
  SET(CMAKE_MFC_FLAG 2)
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:\"mainCRTStartup\" ")
ENDIF(WIN32)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${GUI} main.cpp ../resources_source/resources.rc ${RESOURCES_RCC})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_QTMAIN_LIBRARY} ${PYTHON_LIBRARY} ${QTAWESOME_LIBRARY})

QT5_USE_MODULES(${PROJECT_NAME} Core Widgets Network Xml XmlPatterns WebKit WebKitWidgets UiTools Svg PrintSupport)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME pythonlab)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
