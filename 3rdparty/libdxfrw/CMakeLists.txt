PROJECT(${DXFLIB_LIBRARY})
  
SET(SOURCES  drw_entities.cpp
    drw_objects.cpp
    libdxfrw.cpp
    intern/drw_textcodec.cpp
    intern/dxfreader.cpp
    intern/dxfwriter.cpp)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns)
