#begin static_lib_target
  #define TARGET p3tinyxml

  #define COMBINED_SOURCES tinyxml_composite1.cxx

  #define SOURCES \
     tinyxml.h

  #define INCLUDED_SOURCES  \
     tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  #define EXTRA_CDEFS TIXML_USE_STL
  #define C++FLAGS $[CFLAGS_SHARED]

#end static_lib_target

#begin lib_target
  #define TARGET dxml

  #define LOCAL_LIBS pandabase
  #define OTHER_LIBS \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c prc:c dtool:m

  #define COMBINED_SOURCES dxml_composite1.cxx

  #define SOURCES \
    config_dxml.h tinyxml.h

  #define INCLUDED_SOURCES \
    config_dxml.cxx \
    tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  #define IGATESCAN all
#end lib_target
