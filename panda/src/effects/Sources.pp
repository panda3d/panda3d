#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET effects
  
  #define LOCAL_LIBS \
    display sgraph graph sgraphutil sgattrib gobj putil gsgbase linmath \
    mathutil switchnode
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx         

  #define SOURCES \
    config_effects.h lensFlareNode.I lensFlareNode.h
    
  #define INCLUDED_SOURCES \
    config_effects.cxx lensFlareNode.cxx 

  #define INSTALL_HEADERS \
    config_effects.h lensFlareNode.I lensFlareNode.h

  #define IGATESCAN all

#end lib_target

