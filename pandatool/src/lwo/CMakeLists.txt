set(P3LWO_HEADERS
  config_lwo.h
  iffChunk.h iffChunk.I
  iffGenericChunk.h iffGenericChunk.I
  iffId.h iffId.I
  iffInputFile.h iffInputFile.I
  lwoBoundingBox.h
  lwoChunk.h
  lwoClip.h
  lwoDiscontinuousVertexMap.h
  lwoGroupChunk.h
  lwoHeader.h lwoHeader.I
  lwoInputFile.h lwoInputFile.I
  lwoLayer.h
  lwoPoints.h
  lwoPolygons.h lwoPolygonTags.h
  lwoStillImage.h
  lwoSurfaceBlockAxis.h lwoSurfaceBlockChannel.h lwoSurfaceBlockCoordSys.h
  lwoSurfaceBlockEnabled.h lwoSurfaceBlock.h lwoSurfaceBlockHeader.h
  lwoSurfaceBlockImage.h lwoSurfaceBlockOpacity.h lwoSurfaceBlockProjection.h
  lwoSurfaceBlockRefObj.h lwoSurfaceBlockRepeat.h lwoSurfaceBlockTMap.h
  lwoSurfaceBlockTransform.h lwoSurfaceBlockVMapName.h lwoSurfaceBlockWrap.h
  lwoSurfaceColor.h
  lwoSurface.h
  lwoSurfaceParameter.h
  lwoSurfaceSidedness.h
  lwoSurfaceSmoothingAngle.h
  lwoTags.h
  lwoVertexMap.h
)

set(P3LWO_SOURCES
  config_lwo.cxx
  iffChunk.cxx
  iffGenericChunk.cxx
  iffId.cxx
  iffInputFile.cxx
  lwoBoundingBox.cxx
  lwoChunk.cxx
  lwoClip.cxx
  lwoDiscontinuousVertexMap.cxx
  lwoGroupChunk.cxx
  lwoHeader.cxx
  lwoInputFile.cxx
  lwoLayer.cxx
  lwoPoints.cxx
  lwoPolygons.cxx
  lwoPolygonTags.cxx
  lwoStillImage.cxx
  lwoSurfaceBlockAxis.cxx lwoSurfaceBlockChannel.cxx lwoSurfaceBlockCoordSys.cxx
  lwoSurfaceBlock.cxx lwoSurfaceBlockEnabled.cxx lwoSurfaceBlockHeader.cxx
  lwoSurfaceBlockImage.cxx lwoSurfaceBlockOpacity.cxx lwoSurfaceBlockProjection.cxx
  lwoSurfaceBlockRefObj.cxx lwoSurfaceBlockRepeat.cxx lwoSurfaceBlockTMap.cxx
  lwoSurfaceBlockTransform.cxx lwoSurfaceBlockVMapName.cxx lwoSurfaceBlockWrap.cxx
  lwoSurfaceColor.cxx
  lwoSurface.cxx
  lwoSurfaceParameter.cxx
  lwoSurfaceSidedness.cxx
  lwoSurfaceSmoothingAngle.cxx
  lwoTags.cxx
  lwoVertexMap.cxx
)

composite_sources(p3lwo P3LWO_SOURCES)
add_library(p3lwo STATIC ${P3LWO_HEADERS} ${P3LWO_SOURCES})
target_link_libraries(p3lwo p3pandatoolbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
