PROJECT(${QUAZIP_LIBRARY})

SET(SOURCES JlCompress.cpp 
    qioapi.cpp 
    quaadler32.cpp 
    quacrc32.cpp 
    quagzipfile.cpp 
    quaziodevice.cpp 
    quazip.cpp 
    quazipdir.cpp 
    quazipfile.cpp 
    quazipnewinfo.cpp 
    quazipfileinfo.cpp
    unzip.c 
    zip.c
  )
  
SET(HEADERS crypt.h
    ioapi.h
    JlCompress.h
    quaadler32.h
    quachecksum32.h
    quacrc32.h
    quagzipfile.h
    quaziodevice.h
    quazipdir.h
    quazipfile.h
    quazipfileinfo.h
    quazip_global.h
    quazip.h
    quazipnewinfo.h
  )
  
IF (AGROS_BUILD_STATIC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
ENDIF()

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})
QT5_USE_MODULES(${PROJECT_NAME} Core)
