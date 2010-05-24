#define LOCAL_LIBS p3tinyxml pandabase

#begin lib_target
  #define TARGET dxml

  #define OTHER_LIBS \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c prc:c dtool:m

  #define SOURCES \
    config_dxml.h config_dxml.cxx

  #define IGATESCAN all
#end lib_target
