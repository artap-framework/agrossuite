PROJECT(${SSH_LIBRARY})

FIND_PACKAGE(Qt5Network REQUIRED)

SET(SOURCES sftpchannel.cpp          
    sftpoperation.cpp       
    sshchannel.cpp
    sshcryptofacility.cpp     
    sshinit.cpp                  
    sshoutgoingpacket.cpp  
    sshremoteprocessrunner.cpp
    sftpdefs.cpp             
    sftpoutgoingpacket.cpp  
    sshchannelmanager.cpp     
    sshdirecttcpiptunnel.cpp  
    sshkeyexchange.cpp           
    sshpacket.cpp          
    sshsendfacility.cpp
    sftpfilesystemmodel.cpp  
    sftppacket.cpp          
    sshconnection.cpp         
    sshhostkeydatabase.cpp    
    sshkeygenerator.cpp          
    sshpacketparser.cpp
    sftpincomingpacket.cpp   
    sshcapabilities.cpp     
    sshconnectionmanager.cpp  
    sshincomingpacket.cpp     
    sshkeypasswordretriever.cpp  
    sshremoteprocess.cpp
    botan/botan.cpp
  )
  
SET(HEADERS sftpchannel.h
    sftpoperation_p.h
    sshchannelmanager_p.h
    sshcryptofacility_p.h
    ssh_global.h
    sshkeygenerator.h
    sshpseudoterminal.h
    sftpchannel_p.h
    sftpoutgoingpacket_p.h
    sshchannel_p.h
    sshdirecttcpiptunnel.h
    sshhostkeydatabase.h
    sshkeypasswordretriever_p.h
    sshremoteprocess.h
    sftpdefs.h
    sftppacket_p.h
    sshconnection.h
    sshdirecttcpiptunnel_p.h
    sshincomingpacket_p.h
    sshoutgoingpacket_p.h
    sshremoteprocess_p.h
    sftpfilesystemmodel.h
    sshbotanconversions_p.h
    sshconnectionmanager.h
    ssherrors.h
    sshinit_p.h
    sshpacketparser_p.h
    sshremoteprocessrunner.h
    sftpincomingpacket_p.h
    sshcapabilities_p.h
    sshconnection_p.h
    sshexception_p.h
    sshkeyexchange_p.h
    sshpacket_p.h
    sshsendfacility_p.h
    botan/botan.h
  )
  
# BOTAN_HAS_MUTEX_WIN32
ADD_LIBRARY(${PROJECT_NAME} ${SOURCES} ${HEADERS})
QT5_USE_MODULES(${PROJECT_NAME} Core Network Gui Widgets)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_LIBRARIES})

IF(WIN32)
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE
    BOTAN_TARGET_OS_IS_WINDOWS
    BOTAN_TARGET_OS_HAS_LOADLIBRARY
    BOTAN_TARGET_OS_HAS_WIN32_GET_SYSTEMTIME
    BOTAN_TARGET_OS_HAS_WIN32_VIRTUAL_LOCK
    BOTAN_HAS_DYNAMICALLY_LOADED_ENGINE
    BOTAN_HAS_DYNAMIC_LOADER
    BOTAN_HAS_ENTROPY_SRC_CAPI
    BOTAN_HAS_ENTROPY_SRC_WIN32
    BOTAN_HAS_MUTEX_WIN32)
ELSE(WIN32)
    TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE
    BOTAN_TARGET_OS_HAS_GETTIMEOFDAY
    BOTAN_HAS_ALLOC_MMAP
    BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM
    BOTAN_HAS_ENTROPY_SRC_EGD
    BOTAN_HAS_ENTROPY_SRC_FTW
    BOTAN_HAS_ENTROPY_SRC_UNIX
    BOTAN_HAS_MUTEX_PTHREAD
    BOTAN_HAS_PIPE_UNIXFD_IO)
ENDIF(WIN32)




