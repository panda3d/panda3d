#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#define USE_PACKAGES 
#define BUILDING_DLL BUILDING_PANDASKEL

#begin lib_target
  #define TARGET p3skel
  #define LOCAL_LIBS \
    p3display p3text p3pgraph p3gobj p3linmath p3putil
    
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

