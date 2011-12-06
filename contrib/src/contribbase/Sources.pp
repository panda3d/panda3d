#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m  p3prc:c

#begin lib_target
  #define TARGET p3contribbase

  #define SOURCES \
    contribbase.cxx contribbase.h contribsymbols.h \

  #define INSTALL_HEADERS \
    contribbase.h contribbase.h

#end lib_target
