#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define LOCAL_LIBS \
    p3event p3gsgbase p3gobj p3putil p3linmath \
    p3downloader p3express p3pandabase p3pstatclient p3pgraph
#define USE_PACKAGES python
 
#begin lib_target
  #define TARGET p3pgraphnodes

  #define COMBINED_SOURCES \
    $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    ambientLight.h ambientLight.I \
    callbackNode.h callbackNode.I \
    computeNode.h computeNode.I \
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
    computeNode.cxx \
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
    computeNode.h computeNode.I \
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
