#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET light
  #define LOCAL_LIBS \
    putil display graph sgraph gobj sgattrib pnmimage mathutil gsgbase \
    linmath
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    
  #define SOURCES \
    ambientLight.I ambientLight.h \
    config_light.h \
    directionalLight.I directionalLight.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightLensNode.I lightLensNode.h \
    lightNode.I lightNode.h \
    lightTransition.I lightTransition.h \
    pointLight.I pointLight.h \
    pt_Light.h \
    spotlight.I spotlight.h \
    vector_PT_Light.h 
    
  #define INCLUDED_SOURCES \
    ambientLight.cxx \
    config_light.cxx \
    directionalLight.cxx \
    light.cxx \
    lightAttrib.cxx \
    lightLensNode.cxx \
    lightNode.cxx \
    lightTransition.cxx \
    pointLight.cxx \
    pt_Light.cxx \
    spotlight.cxx \
    vector_PT_Light.cxx 

  #define INSTALL_HEADERS \
    ambientLight.I ambientLight.h \
    directionalLight.I directionalLight.h \
    light.I light.h \
    lightAttrib.I lightAttrib.h \
    lightLensNode.I lightLensNode.h \
    lightNode.I lightNode.h \
    lightNameClass.h \
    lightTransition.I lightTransition.h \
    pointLight.I pointLight.h \
    pt_Light.h \
    spotlight.I spotlight.h \
    vector_PT_Light.h

  #define IGATESCAN all

#end lib_target

