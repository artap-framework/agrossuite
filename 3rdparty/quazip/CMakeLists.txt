PROJECT(${QUAZIP_LIBRARY})
# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIRS}
)

set(HEADERS
        JlCompress.h
        ioapi.h
        minizip_crypt.h
        quaadler32.h
        quachecksum32.h
        quacrc32.h
        quagzipfile.h
        quaziodevice.h
        quazip.h
        quazip_global.h
        quazipdir.h
        quazipfile.h
        quazipfileinfo.h
        quazipnewinfo.h
        unzip.h
        zip.h
   )

set(SOURCES
        ${QUAZIP_HEADERS}
        unzip.c
        zip.c
        JlCompress.cpp
        qioapi.cpp
        quaadler32.cpp
        quachecksum32.cpp
        quacrc32.cpp
        quagzipfile.cpp
        quaziodevice.cpp
        quazip.cpp
        quazipdir.cpp
        quazipfile.cpp
        quazipfileinfo.cpp
        quazipnewinfo.cpp
   )

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

qt_wrap_cpp(MOC_SRCS ${HEADERS})
set(SRCS ${SRCS} ${MOC_SRCS})

add_library(${QUAZIP_LIB_TARGET_NAME} SHARED ${SRCS})
# add_library(quazip_static STATIC ${SRCS})
 
IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
ENDIF()
 
FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES} ${ZLIB_LIBRARIES})
QT5_USE_MODULES(${PROJECT_NAME} Core)

