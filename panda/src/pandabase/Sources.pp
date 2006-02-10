#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m  prc:c

#begin lib_target
  #define TARGET pandabase
  
  #define SOURCES \
    pandabase.cxx pandabase.h pandasymbols.h \

  #define INSTALL_HEADERS \
    pandabase.h pandasymbols.h

#end lib_target
