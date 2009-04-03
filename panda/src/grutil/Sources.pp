#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES opencv ffmpeg artoolkit

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil movies audio
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES \
    pipeOcclusionCullTraverser.I pipeOcclusionCullTraverser.h \
    cardMaker.I cardMaker.h \
    arToolKit.I arToolKit.h \
    config_grutil.h \
    ffmpegTexture.I ffmpegTexture.h \
    movieTexture.I movieTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    meshDrawer.I meshDrawer.h \
    geoMipTerrain.I geoMipTerrain.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h multitexReducer.cxx \
    nodeVertexTransform.I nodeVertexTransform.h \
    openCVTexture.I openCVTexture.h \
    rigidBodyCombiner.I rigidBodyCombiner.h
    
  #define INCLUDED_SOURCES \
    cardMaker.cxx \
    arToolKit.cxx \
    ffmpegTexture.cxx \
    movieTexture.cxx \
    fisheyeMaker.cxx \
    config_grutil.cxx \
    frameRateMeter.cxx \
    meshDrawer.cxx \
    geoMipTerrain.cxx \
    heightfieldTesselator.cxx \
    nodeVertexTransform.cxx \
    openCVTexture.cxx \    	 
    pipeOcclusionCullTraverser.cxx \
    lineSegs.cxx \
    rigidBodyCombiner.cxx
    
  #define INSTALL_HEADERS \
    cardMaker.I cardMaker.h \
    arToolKit.I arToolKit.h \
    ffmpegTexture.I ffmpegTexture.h \
    movieTexture.I movieTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    meshDrawer.I meshDrawer.h \
    geoMipTerrain.I geoMipTerrain.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h \
    nodeVertexTransform.I nodeVertexTransform.h \
    openCVTexture.I openCVTexture.h \
    rigidBodyCombiner.I rigidBodyCombiner.h

  #define IGATESCAN all

#end lib_target

