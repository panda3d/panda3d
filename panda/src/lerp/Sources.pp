#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET lerp
  #define LOCAL_LIBS \
    event linmath putil

  #define SOURCES \
    config_lerp.cxx config_lerp.h lerp.cxx lerp.h lerpblend.cxx \
    lerpblend.h lerpfunctor.cxx lerpfunctor.h

  #define INSTALL_HEADERS \
    lerp.h lerpblend.h lerpfunctor.h

  #define IGATESCAN all

#end lib_target

