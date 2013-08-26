#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3grutil
  #define LOCAL_LIBS \
    p3display p3text p3pgraph p3gobj p3linmath p3putil p3movies p3audio
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    pipeOcclusionCullTraverser.I pipeOcclusionCullTraverser.h \
    cardMaker.I cardMaker.h \
    config_grutil.h \
    movieTexture.I movieTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    meshDrawer.I meshDrawer.h \
    meshDrawer2D.I meshDrawer2D.h \
    geoMipTerrain.I geoMipTerrain.h \
    sceneGraphAnalyzerMeter.I sceneGraphAnalyzerMeter.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h multitexReducer.cxx \
    nodeVertexTransform.I nodeVertexTransform.h \
    pfmVizzer.I pfmVizzer.h \
    rigidBodyCombiner.I rigidBodyCombiner.h
    
  #define INCLUDED_SOURCES \
    cardMaker.cxx \
    movieTexture.cxx \
    fisheyeMaker.cxx \
    config_grutil.cxx \
    frameRateMeter.cxx \
    meshDrawer.cxx \
    meshDrawer2D.cxx \
    geoMipTerrain.cxx \
    sceneGraphAnalyzerMeter.cxx \
    heightfieldTesselator.cxx \
    nodeVertexTransform.cxx \    
    pfmVizzer.cxx \
    pipeOcclusionCullTraverser.cxx \
    lineSegs.cxx \
    rigidBodyCombiner.cxx
    
  #define INSTALL_HEADERS \
    cardMaker.I cardMaker.h \
    movieTexture.I movieTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    meshDrawer.I meshDrawer.h \
    meshDrawer2D.I meshDrawer2D.h \
    geoMipTerrain.I geoMipTerrain.h \
    sceneGraphAnalyzerMeter.I sceneGraphAnalyzerMeter.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h \
    nodeVertexTransform.I nodeVertexTransform.h \
    pfmVizzer.I pfmVizzer.h \
    rigidBodyCombiner.I rigidBodyCombiner.h

  #define IGATESCAN all

#end lib_target

