#define OTHER_LIBS interrogatedb dconfig dtoolutil

#begin lib_target
  #define TARGET pandabase
  
  #define SOURCES \
    pandabase.cxx pandabase.h pandasymbols.h \

  #define INSTALL_HEADERS \
    pandabase.h pandasymbols.h

#end lib_target
