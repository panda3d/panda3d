#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET text
  #define LOCAL_LIBS \
    cull putil gobj sgattrib graph sgraph linmath sgraphutil pnmimage gsgbase \
    mathutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_text.h textFont.I textFont.h \
    textNode.I textNode.h textNode.cxx

  #define INCLUDED_SOURCES config_text.cxx textFont.cxx

  #define INSTALL_HEADERS \
    config_text.h textFont.I textFont.h textNode.I textNode.h

  #define IGATESCAN all

#end lib_target

