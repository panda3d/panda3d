#define INSTALL_HEADERS \
    Makefile.a2egg-static.rules Makefile.a2egg.rules \
    Makefile.char-egg.rules Makefile.findlatest Makefile.flt-egg.rules \
    Makefile.inst-egg.vars Makefile.soft2egg.rules \
    Makefile.source-egg.rules Makefile.textures.rules

#begin bin_target
  #define TARGET findlatest
  #define SOURCES findlatest.cxx
  #define LOCAL_LIBS dtoolbase
#end bin_target
