#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
//#define OSX_SYS_LIBS mx

#begin lib_target
  #define TARGET p3distort
  #define LOCAL_LIBS \
    p3display p3pgraph p3gobj p3linmath
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES config_distort.h \
    projectionScreen.h projectionScreen.I \
    cylindricalLens.h cylindricalLens.I \
    fisheyeLens.h fisheyeLens.I \
    nonlinearImager.h nonlinearImager.I \
    oSphereLens.h oSphereLens.I \
    pSphereLens.h pSphereLens.I

  #define INCLUDED_SOURCES \
    config_distort.cxx cylindricalLens.cxx fisheyeLens.cxx nonlinearImager.cxx \
    projectionScreen.cxx oSphereLens.cxx pSphereLens.cxx

  #define INSTALL_HEADERS

  #define IGATESCAN all

#end lib_target

