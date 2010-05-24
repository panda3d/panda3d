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
