PROJECT(${QUAZIP_LIBRARY})
# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIR}
)

set(QUAZIP_HEADERS
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
        quazip_qt_compat.h
        quazipdir.h
        quazipfile.h
        quazipfileinfo.h
        quazipnewinfo.h
        unzip.h
        zip.h
   )

set(QUAZIP_SOURCES
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

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

FIND_PACKAGE(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Core5Compat REQUIRED)

add_library(${QUAZIP_LIB_TARGET_NAME} SHARED ${SRCS})
# add_library(quazip_static STATIC ${SRCS})

IF (MSVC)
    ADD_LIBRARY(${PROJECT_NAME} STATIC ${HEADERS} ${QUAZIP_SOURCES})
ELSE()
    ADD_LIBRARY(${PROJECT_NAME} SHARED ${HEADERS} ${QUAZIP_SOURCES})
ENDIF()
 
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ZLIB_LIBRARIES} Qt6::Core Qt6::Core5Compat)


