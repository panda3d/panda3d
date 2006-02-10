#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET lerp
  #define LOCAL_LIBS \
    event linmath putil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_lerp.h lerp.h lerpblend.h lerpfunctor.h
    
  #define INCLUDED_SOURCES \
    config_lerp.cxx lerp.cxx lerpblend.cxx lerpfunctor.cxx 

  #define INSTALL_HEADERS \
    lerp.h lerpblend.h lerpfunctor.h
    
  #define IGATESCAN all

#end lib_target

