#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET light
  #define LOCAL_LIBS \
    putil display graph sgraph gobj sgattrib pnmimage mathutil gsgbase \
    linmath

  #define SOURCES \
    ambientLight.cxx ambientLight.h config_light.cxx config_light.h \
    directionalLight.I directionalLight.cxx directionalLight.h \
    light.cxx light.h lightAttribute.I lightAttribute.cxx \
    lightAttribute.h lightTransition.I lightTransition.cxx \
    lightTransition.h pointLight.I pointLight.cxx pointLight.h \
    pt_Light.cxx pt_Light.h spotlight.I spotlight.cxx spotlight.h \
    vector_PT_Light.cxx vector_PT_Light.h

  #define INSTALL_HEADERS \
    ambientLight.h directionalLight.I directionalLight.h light.h \
    lightAttribute.I lightAttribute.h lightNameClass.h \
    lightTransition.I lightTransition.h pointLight.I pointLight.h \
    pt_Light.h spotlight.I spotlight.h vector_PT_Light.h

  #define IGATESCAN all

#end lib_target

