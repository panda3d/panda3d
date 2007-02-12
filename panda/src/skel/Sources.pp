#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES 
#define BUILDING_DLL BUILDING_PANDASKEL

#begin lib_target
  #define TARGET skel
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_skel.h \
    basicSkel.I basicSkel.h \
    typedSkel.I typedSkel.h
    
  #define INCLUDED_SOURCES \
    config_skel.cxx \
    basicSkel.cxx \
    typedSkel.cxx

  #define INSTALL_HEADERS \
    basicSkel.h basicSkel.I \
    typedSkel.I typedSkel.h

  #define IGATESCAN all

#end lib_target

