#define YACC_PREFIX vrmlyy

#begin ss_lib_target
  #define TARGET pvrml
  #define LOCAL_LIBS \
    pandatoolbase
  #define OTHER_LIBS \
    mathutil:c linmath:c pipeline:c \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m

  #define USE_PACKAGES zlib

  #define SOURCES \
    parse_vrml.cxx parse_vrml.h \
    standard_nodes.cxx standard_nodes.h \
    vrmlLexer.lxx \
    vrmlParser.yxx \
    vrmlNode.cxx vrmlNode.h \
    vrmlNodeType.cxx vrmlNodeType.h

#end ss_lib_target
