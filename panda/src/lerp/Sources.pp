#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3lerp
  #define LOCAL_LIBS \
    p3event p3linmath p3putil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_lerp.h lerp.h lerpblend.h lerpfunctor.h
    
  #define INCLUDED_SOURCES \
    config_lerp.cxx lerp.cxx lerpblend.cxx lerpfunctor.cxx 

  #define INSTALL_HEADERS \
    lerp.h lerpblend.h lerpfunctor.h
    
  #define IGATESCAN all

#end lib_target

