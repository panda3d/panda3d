#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m  p3prc:c

#begin lib_target
  #define TARGET p3pandabase
  
  #define SOURCES \
    pandabase.cxx pandabase.h pandasymbols.h \

  #define INSTALL_HEADERS \
    pandabase.h pandasymbols.h

#end lib_target
