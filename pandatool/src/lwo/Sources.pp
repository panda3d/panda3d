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
    lwoBoundingBox.cxx lwoBoundingBox.h \
    lwoChunk.cxx lwoChunk.h \
    lwoClip.cxx lwoClip.h \
    lwoDiscontinuousVertexMap.cxx lwoDiscontinuousVertexMap.h \
    lwoGroupChunk.cxx lwoGroupChunk.h \
    lwoHeader.cxx \
    lwoHeader.h lwoInputFile.cxx lwoInputFile.h \
    lwoLayer.h lwoLayer.cxx \
    lwoPoints.h lwoPoints.cxx \
    lwoPolygons.h lwoPolygons.cxx \
    lwoPolygonTags.h lwoPolygonTags.cxx \
    lwoTags.h lwoTags.cxx \
    lwoStillImage.h lwoStillImage.cxx \
    lwoSurface.h lwoSurface.cxx \
    lwoSurfaceColor.h lwoSurfaceColor.cxx \
    lwoSurfaceParameter.h lwoSurfaceParameter.cxx \
    lwoVertexMap.h lwoVertexMap.cxx

  #define INSTALL_HEADERS \
    iffChunk.I iffChunk.h iffGenericChunk.I iffGenericChunk.h iffId.I \
    iffId.h iffInputFile.I iffInputFile.h \
    lwoBoundingBox.h \
    lwoChunk.h \
    lwoClip.h \
    lwoDiscontinuousVertexMap.h \
    lwoGroupChunk.h \
    lwoHeader.h \
    lwoInputFile.h \
    lwoLayer.h \
    lwoPoints.h \
    lwoPolygons.h \
    lwoPolygonTags.h \
    lwoTags.h \
    lwoStillImage.h \
    lwoSurface.h \
    lwoSurfaceColor.h \
    lwoSurfaceParameter.h \
    lwoVertexMap.h

#end ss_lib_target

#begin test_bin_target
  #define TARGET test_lwo
  #define LOCAL_LIBS lwo $[LOCAL_LIBS]
  #define OTHER_LIBS $[OTHER_LIBS] pystub:c

  #define SOURCES \
    test_lwo.cxx

#end test_bin_target
