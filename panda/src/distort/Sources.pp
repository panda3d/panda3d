#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define OSX_SYS_LIBS mx

#begin lib_target
  #define TARGET distort
  #define LOCAL_LIBS \
    display pgraph gobj linmath
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES config_distort.h \
    projectionScreen.h projectionScreen.I \
    cylindricalLens.h cylindricalLens.I \
    fisheyeLens.h fisheyeLens.I \
    nonlinearImager.h nonlinearImager.I \
    pSphereLens.h pSphereLens.I

  #define INCLUDED_SOURCES \
    config_distort.cxx cylindricalLens.cxx fisheyeLens.cxx nonlinearImager.cxx \
    projectionScreen.cxx pSphereLens.cxx 

  #define INSTALL_HEADERS

  #define IGATESCAN all

#end lib_target

