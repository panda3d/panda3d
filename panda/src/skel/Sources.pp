#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES 

#begin lib_target
  #define TARGET skel
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_skel.h \
    basicSkel.I basicSkel.h
    
  #define INCLUDED_SOURCES \
    config_skel.cxx \
    basicSkel.cxx

  #define INSTALL_HEADERS \
    basicSkel.h basicSkel.I

  #define IGATESCAN all

#end lib_target

