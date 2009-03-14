#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define LOCAL_LIBS \
    lerp event gsgbase gobj putil linmath \
    downloader express pandabase pstatclient pgraph
#define USE_PACKAGES python
 
#begin lib_target
  #define TARGET pgraphnodes

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
    lightLensNode.h lightLensNode.I \
    lightNode.h lightNode.I \
    nodeCullCallbackData.h nodeCullCallbackData.I \
    pointLight.h pointLight.I \
    selectiveChildNode.h selectiveChildNode.I \
    sequenceNode.h sequenceNode.I \
    shaderGenerator.h shaderGenerator.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I

  #define INCLUDED_SOURCES \
    ambientLight.cxx \
    callbackNode.cxx \
    config_pgraphnodes.cxx \
    directionalLight.cxx \
    lightLensNode.cxx \
    lightNode.cxx \
    nodeCullCallbackData.cxx \
    pointLight.cxx \
    selectiveChildNode.cxx \
    sequenceNode.cxx \
    shaderGenerator.cxx \
    spotlight.cxx \
    switchNode.cxx

  #define INSTALL_HEADERS \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
    lightLensNode.h lightLensNode.I \
    lightNode.h lightNode.I \
    nodeCullCallbackData.h nodeCullCallbackData.I \
    pointLight.h pointLight.I \
    selectiveChildNode.h selectiveChildNode.I \
    sequenceNode.h sequenceNode.I \
    shaderGenerator.h shaderGenerator.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I

  #define IGATESCAN all

#end lib_target
