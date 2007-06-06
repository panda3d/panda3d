#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define USE_PACKAGES opencv ffmpeg

#begin lib_target
  #define TARGET grutil
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    pipeOcclusionCullTraverser.I pipeOcclusionCullTraverser.h \
    cardMaker.I cardMaker.h \
    config_grutil.h \
    ffmpegTexture.I ffmpegTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h multitexReducer.cxx \
    nodeVertexTransform.I nodeVertexTransform.h \
    openCVTexture.I openCVTexture.h \
    rigidBodyCombiner.I rigidBodyCombiner.h
    
  #define INCLUDED_SOURCES \
    cardMaker.cxx \
    ffmpegTexture.cxx \
    fisheyeMaker.cxx \
    config_grutil.cxx \
    frameRateMeter.cxx \
    heightfieldTesselator.cxx \
    nodeVertexTransform.cxx \
    openCVTexture.cxx \    	 
    pipeOcclusionCullTraverser.cxx \
    lineSegs.cxx \
    rigidBodyCombiner.cxx
    

  #define INSTALL_HEADERS \
    cardMaker.I cardMaker.h \
    ffmpegTexture.I ffmpegTexture.h \
    fisheyeMaker.I fisheyeMaker.h \
    frameRateMeter.I frameRateMeter.h \
    heightfieldTesselator.I heightfieldTesselator.h \
    lineSegs.I lineSegs.h \
    multitexReducer.I multitexReducer.h \
    nodeVertexTransform.I nodeVertexTransform.h \
    openCVTexture.I openCVTexture.h \
    rigidBodyCombiner.I rigidBodyCombiner.h

  #define IGATESCAN all

#end lib_target

