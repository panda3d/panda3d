#define LOCAL_LIBS pandatoolbase
#define OTHER_LIBS \
  mathutil:c linmath:c putil:c express:c panda:m pandaexpress:m dtoolconfig dtool
#define UNIX_SYS_LIBS \
  m

#begin ss_lib_target
  #define TARGET lwo

  #define SOURCES \
    config_lwo.cxx config_lwo.h iffChunk.I iffChunk.cxx iffChunk.h \
    iffGenericChunk.I iffGenericChunk.cxx iffGenericChunk.h iffId.I \
    iffId.cxx iffId.h iffInputFile.I iffInputFile.cxx iffInputFile.h \
    lwoBoundingBox.cxx lwoBoundingBox.h \
    lwoChunk.cxx lwoChunk.h \
    lwoClip.cxx lwoClip.h \
    lwoDiscontinuousVertexMap.cxx lwoDiscontinuousVertexMap.h \
    lwoGroupChunk.cxx lwoGroupChunk.h \
    lwoHeader.I lwoHeader.cxx lwoHeader.h \
    lwoInputFile.I lwoInputFile.cxx lwoInputFile.h \
    lwoLayer.h lwoLayer.cxx \
    lwoPoints.h lwoPoints.cxx \
    lwoPolygons.h lwoPolygons.cxx \
    lwoPolygonTags.h lwoPolygonTags.cxx \
    lwoTags.h lwoTags.cxx \
    lwoStillImage.h lwoStillImage.cxx \
    lwoSurface.h lwoSurface.cxx \
    lwoSurfaceBlock.h lwoSurfaceBlock.cxx \
    lwoSurfaceBlockAxis.h lwoSurfaceBlockAxis.cxx \
    lwoSurfaceBlockChannel.h lwoSurfaceBlockChannel.cxx \
    lwoSurfaceBlockCoordSys.h lwoSurfaceBlockCoordSys.cxx \
    lwoSurfaceBlockEnabled.h lwoSurfaceBlockEnabled.cxx \
    lwoSurfaceBlockImage.h lwoSurfaceBlockImage.cxx \
    lwoSurfaceBlockOpacity.h lwoSurfaceBlockOpacity.cxx \
    lwoSurfaceBlockProjection.h lwoSurfaceBlockProjection.cxx \
    lwoSurfaceBlockHeader.h lwoSurfaceBlockHeader.cxx \
    lwoSurfaceBlockRefObj.h lwoSurfaceBlockRefObj.cxx \
    lwoSurfaceBlockRepeat.h lwoSurfaceBlockRepeat.cxx \
    lwoSurfaceBlockTMap.h lwoSurfaceBlockTMap.cxx \
    lwoSurfaceBlockTransform.h lwoSurfaceBlockTransform.cxx \
    lwoSurfaceBlockWrap.h lwoSurfaceBlockWrap.cxx \
    lwoSurfaceColor.h lwoSurfaceColor.cxx \
    lwoSurfaceParameter.h lwoSurfaceParameter.cxx \
    lwoSurfaceSidedness.h lwoSurfaceSidedness.cxx \
    lwoSurfaceSmoothingAngle.h lwoSurfaceSmoothingAngle.cxx \
    lwoVertexMap.h lwoVertexMap.cxx

  #define INSTALL_HEADERS \
    iffChunk.I iffChunk.h iffGenericChunk.I iffGenericChunk.h iffId.I \
    iffId.h iffInputFile.I iffInputFile.h \
    lwoBoundingBox.h \
    lwoChunk.h \
    lwoClip.h \
    lwoDiscontinuousVertexMap.h \
    lwoGroupChunk.h \
    lwoHeader.I lwoHeader.h \
    lwoInputFile.I lwoInputFile.h \
    lwoLayer.h \
    lwoPoints.h \
    lwoPolygons.h \
    lwoPolygonTags.h \
    lwoTags.h \
    lwoStillImage.h \
    lwoSurface.h \
    lwoSurfaceBlock.h \
    lwoSurfaceBlockAxis.h \
    lwoSurfaceBlockChannel.h \
    lwoSurfaceBlockCoordSys.h \
    lwoSurfaceBlockEnabled.h \
    lwoSurfaceBlockImage.h \
    lwoSurfaceBlockOpacity.h \
    lwoSurfaceBlockProjection.h \
    lwoSurfaceBlockHeader.h \
    lwoSurfaceBlockRefObj.h \
    lwoSurfaceBlockRepeat.h \
    lwoSurfaceBlockTMap.h \
    lwoSurfaceBlockTransform.h \
    lwoSurfaceBlockWrap.h \
    lwoSurfaceColor.h \
    lwoSurfaceParameter.h \
    lwoSurfaceSidedness.h \
    lwoSurfaceSmoothingAngle.h \
    lwoVertexMap.h

#end ss_lib_target
