PROJECT(${DXFLIB_LIBRARY})
  
SET(SOURCES drw_classes.cpp
    drw_entities.cpp
    drw_header.cpp
    drw_objects.cpp
    libdwgr.cpp
    libdxfrw.cpp
    intern/drw_dbg.cpp
    intern/drw_textcodec.cpp
    intern/dwgbuffer.cpp
    intern/dwgreader15.cpp
    intern/dwgreader18.cpp
    intern/dwgreader21.cpp
    intern/dwgreader24.cpp
    intern/dwgreader27.cpp
    intern/dwgreader.cpp
    intern/dwgutil.cpp
    intern/dxfreader.cpp
    intern/dxfwriter.cpp
    intern/rscodec.cpp
    intern/dxfreader.cpp
    intern/dxfwriter.cpp)

IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
ENDIF()

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
