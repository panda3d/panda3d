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
    fadeLodNode.I fadeLodNode.h fadeLodNodeData.h \
    lightLensNode.h lightLensNode.I \
    lightNode.h lightNode.I \
    lodNode.I lodNode.h lodNodeType.h \
    nodeCullCallbackData.h nodeCullCallbackData.I \
    pointLight.h pointLight.I \
    sceneGraphAnalyzer.h sceneGraphAnalyzer.I \
    selectiveChildNode.h selectiveChildNode.I \
    sequenceNode.h sequenceNode.I \
    shaderGenerator.h shaderGenerator.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I \
    uvScrollNode.I uvScrollNode.h

  #define INCLUDED_SOURCES \
    ambientLight.cxx \
    callbackNode.cxx \
    config_pgraphnodes.cxx \
    directionalLight.cxx \
    fadeLodNode.cxx fadeLodNodeData.cxx \
    lightLensNode.cxx \
    lightNode.cxx \
    lodNode.cxx lodNodeType.cxx \
    nodeCullCallbackData.cxx \
    pointLight.cxx \
    sceneGraphAnalyzer.cxx \
    selectiveChildNode.cxx \
    sequenceNode.cxx \
    shaderGenerator.cxx \
    spotlight.cxx \
    switchNode.cxx \
    uvScrollNode.cxx

  #define INSTALL_HEADERS \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    config_pgraphnodes.h \
    directionalLight.h directionalLight.I \
    fadeLodNode.I fadeLodNode.h fadeLodNodeData.h \
    lightLensNode.h lightLensNode.I \
    lightNode.h lightNode.I \
    lodNode.I lodNode.h lodNodeType.h \
    nodeCullCallbackData.h nodeCullCallbackData.I \
    pointLight.h pointLight.I \
    sceneGraphAnalyzer.h sceneGraphAnalyzer.I \
    selectiveChildNode.h selectiveChildNode.I \
    sequenceNode.h sequenceNode.I \
    shaderGenerator.h shaderGenerator.I \
    spotlight.h spotlight.I \
    switchNode.h switchNode.I \
    uvScrollNode.I uvScrollNode.h

  #define IGATESCAN all

#end lib_target
