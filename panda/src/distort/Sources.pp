#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET distort
  #define LOCAL_LIBS \
    sgraphutil sgraph sgattrib gobj linmath

  #define SOURCES \
    config_distort.cxx config_distort.h \
    cylindricalLens.cxx cylindricalLens.h cylindricalLens.I \
    fisheyeLens.cxx fisheyeLens.h fisheyeLens.I \
    nonlinearImager.cxx nonlinearImager.h nonlinearImager.I \
    pSphereLens.cxx pSphereLens.h pSphereLens.I \
    projectionScreen.cxx projectionScreen.h projectionScreen.I

  #define INSTALL_HEADERS

  #define IGATESCAN all

#end lib_target

