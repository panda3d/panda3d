#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m  prc:c

#begin lib_target
  #define TARGET contribbase

  #define SOURCES \
    contribbase.cxx contribbase.h contribsymbols.h \

  #define INSTALL_HEADERS \
    contribbase.h contribbase.h

#end lib_target
