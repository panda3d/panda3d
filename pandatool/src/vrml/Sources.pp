#define YACC_PREFIX vrmlyy

#begin ss_lib_target
  #define TARGET p3vrml
  #define LOCAL_LIBS \
    p3pandatoolbase
  #define OTHER_LIBS \
    p3mathutil:c p3linmath:c p3pipeline:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m

  #define USE_PACKAGES zlib

  #define SOURCES \
    parse_vrml.cxx parse_vrml.h \
    standard_nodes.cxx standard_nodes.h \
    vrmlLexer.lxx \
    vrmlParser.yxx \
    vrmlNode.cxx vrmlNode.h \
    vrmlNodeType.cxx vrmlNodeType.h

#end ss_lib_target
