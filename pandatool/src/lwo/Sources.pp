#define LOCAL_LIBS pandatoolbase
#define OTHER_LIBS \
  mathutil:c linmath:c putil:c express:c panda:m dtoolconfig dtool
#define UNIX_SYS_LIBS \
  m

#begin ss_lib_target
  #define TARGET lwo

  #define SOURCES \
    config_lwo.cxx config_lwo.h iffChunk.I iffChunk.cxx iffChunk.h \
    iffGenericChunk.I iffGenericChunk.cxx iffGenericChunk.h iffId.I \
    iffId.cxx iffId.h iffInputFile.I iffInputFile.cxx iffInputFile.h \
    lwoBoundingBox.I lwoBoundingBox.cxx lwoBoundingBox.h \
    lwoChunk.I lwoChunk.cxx lwoChunk.h \
    lwoClip.I lwoClip.cxx lwoClip.h \
    lwoDiscontinuousVertexMap.I lwoDiscontinuousVertexMap.cxx lwoDiscontinuousVertexMap.h \
    lwoGroupChunk.I lwoGroupChunk.cxx lwoGroupChunk.h \
    lwoHeader.I lwoHeader.cxx \
    lwoHeader.h lwoInputFile.I lwoInputFile.cxx lwoInputFile.h \
    lwoLayer.h lwoLayer.I lwoLayer.cxx \
    lwoPoints.h lwoPoints.I lwoPoints.cxx \
    lwoPolygons.h lwoPolygons.I lwoPolygons.cxx \
    lwoPolygonTags.h lwoPolygonTags.I lwoPolygonTags.cxx \
    lwoTags.h lwoTags.I lwoTags.cxx \
    lwoStillImage.h lwoStillImage.I lwoStillImage.cxx \
    lwoSurface.h lwoSurface.I lwoSurface.cxx \
    lwoSurfaceColor.h lwoSurfaceColor.I lwoSurfaceColor.cxx \
    lwoSurfaceParameter.h lwoSurfaceParameter.I lwoSurfaceParameter.cxx \
    lwoVertexMap.h lwoVertexMap.I lwoVertexMap.cxx

  #define INSTALL_HEADERS \
    iffChunk.I iffChunk.h iffGenericChunk.I iffGenericChunk.h iffId.I \
    iffId.h iffInputFile.I iffInputFile.h \
    lwoBoundingBox.I lwoBoundingBox.h \
    lwoChunk.I lwoChunk.h \
    lwoClip.I lwoClip.h \
    lwoDiscontinuousVertexMap.I lwoDiscontinuousVertexMap.h \
    lwoGroupChunk.I lwoGroupChunk.h \
    lwoHeader.I lwoHeader.h \
    lwoInputFile.I lwoInputFile.h \
    lwoLayer.I lwoLayer.h \
    lwoPoints.I lwoPoints.h \
    lwoPolygons.I lwoPolygons.h \
    lwoPolygonTags.I lwoPolygonTags.h \
    lwoTags.I lwoTags.h \
    lwoStillImage.I lwoStillImage.h \
    lwoSurface.I lwoSurface.h \
    lwoSurfaceColor.I lwoSurfaceColor.h \
    lwoSurfaceParameter.I lwoSurfaceParameter.h \
    lwoVertexMap.I lwoVertexMap.h

#end ss_lib_target

#begin test_bin_target
  #define TARGET test_lwo
  #define LOCAL_LIBS lwo $[LOCAL_LIBS]
  #define OTHER_LIBS $[OTHER_LIBS] pystub:c

  #define SOURCES \
    test_lwo.cxx

#end test_bin_target
