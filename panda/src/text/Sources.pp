#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET text
  #define LOCAL_LIBS \
    putil gobj sgattrib graph sgraph linmath sgraphutil pnmimage gsgbase \
    mathutil

  #define SOURCES \
    config_text.cxx config_text.h textNode.I textNode.cxx textNode.h

  #define INSTALL_HEADERS \
    textNode.I textNode.h

  #define IGATESCAN all

#end lib_target

