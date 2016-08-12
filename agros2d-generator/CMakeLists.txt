PROJECT(${AGROS_GENERATOR})

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate)
IF(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/windows)
ELSE(WIN32)
    INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/ctemplate/linux)
ENDIF(WIN32)

SET(SOURCES main.cpp 
    generator.cpp 
    generator_module.cpp 
    generator_coupling.cpp
    generator_weakforms.cpp
    generator_postprocessor.cpp
    generator_documentation.cpp
    parser.cpp
    parser_instance.cpp
    parser_lexical_analyser.cpp
    parser_module_info.cpp
)

SET(HEADERS generator.h
    generator_module.h
    generator_coupling.h
    parser.h
)

IF(WIN32)
  # XSDCXX
  # message("XML cpp/h files")
  FILE(GLOB generator_xsd_files "${CMAKE_HOME_DIRECTORY}/resources/xsd/*.xsd")
  foreach(XSD_FILE ${generator_xsd_files})
    GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
    ADD_CUSTOM_TARGET(generator-${basefile} ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.cpp ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.h)

    # message("\txsdcxx ${basefile}")

    ADD_CUSTOM_COMMAND(
          COMMAND  ${XSD_BIN} ARGS cxx-tree --std c++11 --generate-ostream --hxx-suffix .h --cxx-suffix .cpp --root-element-first --generate-polymorphic --output-dir ${CMAKE_HOME_DIRECTORY}/resources_source/classes --generate-serialization ${XSD_FILE}
	  DEPENDS ${XSD_FILE}
	  OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.cpp ${CMAKE_HOME_DIRECTORY}/resources_source/classes/${basefile}.h)
  endforeach()

  SET(SOURCES_XML ${CMAKE_HOME_DIRECTORY}/resources_source/classes/module_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/material_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/problem_a2d_31_xml.cpp
    ${CMAKE_HOME_DIRECTORY}/resources_source/classes/form_xml.cpp
)
ENDIF(WIN32)
    
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES} ${SOURCES_XML} ${HEADERS})

IF(WIN32)
    foreach(XSD_FILE ${generator_xsd_files})
      GET_FILENAME_COMPONENT(basefile ${XSD_FILE} NAME_WE)
      ADD_DEPENDENCIES(${PROJECT_NAME} generator-${basefile})
    endforeach()
ENDIF(WIN32)

find_package(Threads)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CTEMPLATE_LIBRARY} ${AGROS_LIBRARY} ${DEAL_II_LIBRARIES} ${QUAZIP_LIBRARY} ${STB_TRUETYPE_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
QT5_USE_MODULES(${PROJECT_NAME} Core Network Xml XmlPatterns)
