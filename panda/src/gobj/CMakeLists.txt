set(P3GOBJ_HEADERS
  adaptiveLru.I adaptiveLru.h
  animateVerticesRequest.I animateVerticesRequest.h
  bufferContext.I bufferContext.h
  bufferContextChain.I bufferContextChain.h
  bufferResidencyTracker.I bufferResidencyTracker.h
  config_gobj.h
  geom.h geom.I
  geomContext.I geomContext.h
  geomEnums.h
  geomMunger.h geomMunger.I
  geomPrimitive.h geomPrimitive.I
  geomPatches.h
  geomTriangles.h
  geomTristrips.h
  geomTrifans.h
  geomLines.h
  geomLinestrips.h
  geomPoints.h
  geomVertexArrayData.h geomVertexArrayData.I
  geomVertexArrayFormat.h geomVertexArrayFormat.I
  geomCacheEntry.h geomCacheEntry.I
  geomCacheManager.h geomCacheManager.I
  geomVertexAnimationSpec.h geomVertexAnimationSpec.I
  geomVertexData.h geomVertexData.I
  geomVertexColumn.h geomVertexColumn.I
  geomVertexFormat.h geomVertexFormat.I
  geomVertexReader.h geomVertexReader.I
  geomVertexRewriter.h geomVertexRewriter.I
  geomVertexWriter.h geomVertexWriter.I
  indexBufferContext.I indexBufferContext.h
  internalName.I internalName.h
  lens.h lens.I
  material.I material.h materialPool.I materialPool.h
  matrixLens.I matrixLens.h
  occlusionQueryContext.I occlusionQueryContext.h
  orthographicLens.I orthographicLens.h
  paramTexture.I paramTexture.h
  perspectiveLens.I perspectiveLens.h
  preparedGraphicsObjects.I preparedGraphicsObjects.h
  queryContext.I queryContext.h
  savedContext.I savedContext.h
  shader.I shader.h
  shaderContext.h shaderContext.I
  simpleAllocator.h simpleAllocator.I
  simpleLru.h simpleLru.I
  sliderTable.I sliderTable.h
  texture.I texture.h
  textureCollection.I textureCollection.h
  textureContext.I textureContext.h
  texturePeeker.I texturePeeker.h
  texturePool.I texturePool.h
  texturePoolFilter.I texturePoolFilter.h
  textureReloadRequest.I textureReloadRequest.h
  textureStage.I textureStage.h
  textureStagePool.I textureStagePool.h
  timerQueryContext.I timerQueryContext.h
  transformBlend.I transformBlend.h
  transformBlendTable.I transformBlendTable.h
  transformTable.I transformTable.h
  userVertexSlider.I userVertexSlider.h
  userVertexTransform.I userVertexTransform.h
  vertexBufferContext.I vertexBufferContext.h
  vertexDataBlock.I vertexDataBlock.h
  vertexDataBook.I vertexDataBook.h
  vertexDataBuffer.I vertexDataBuffer.h
  vertexDataPage.I vertexDataPage.h
  vertexDataSaveFile.I vertexDataSaveFile.h
  vertexSlider.I vertexSlider.h
  vertexTransform.I vertexTransform.h
  videoTexture.I videoTexture.h
)

set(P3GOBJ_SOURCES
  adaptiveLru.cxx
  animateVerticesRequest.cxx
  bufferContext.cxx
  bufferContextChain.cxx
  bufferResidencyTracker.cxx
  config_gobj.cxx
  geomContext.cxx
  geom.cxx
  geomEnums.cxx
  geomMunger.cxx
  geomPrimitive.cxx
  geomPatches.cxx
  geomTriangles.cxx
  geomTristrips.cxx
  geomTrifans.cxx
  geomLines.cxx
  geomLinestrips.cxx
  geomPoints.cxx
  geomVertexArrayData.cxx
  geomVertexArrayFormat.cxx
  geomCacheEntry.cxx
  geomCacheManager.cxx
  geomVertexAnimationSpec.cxx
  geomVertexData.cxx
  geomVertexColumn.cxx
  geomVertexFormat.cxx
  geomVertexReader.cxx
  geomVertexRewriter.cxx
  geomVertexWriter.cxx
  indexBufferContext.cxx
  material.cxx
  internalName.cxx
  lens.cxx
  materialPool.cxx matrixLens.cxx
  occlusionQueryContext.cxx
  orthographicLens.cxx
  paramTexture.cxx
  perspectiveLens.cxx
  preparedGraphicsObjects.cxx
  queryContext.cxx
  samplerContext.cxx
  samplerState.cxx
  savedContext.cxx
  shader.cxx
  shaderContext.cxx
  simpleAllocator.cxx
  simpleLru.cxx
  sliderTable.cxx
  texture.cxx
  textureCollection.cxx
  textureContext.cxx
  texturePeeker.cxx
  texturePool.cxx
  texturePoolFilter.cxx
  textureReloadRequest.cxx
  textureStage.cxx
  textureStagePool.cxx
  timerQueryContext.cxx
  transformBlend.cxx
  transformBlendTable.cxx
  transformTable.cxx
  userVertexSlider.cxx
  userVertexTransform.cxx
  vertexBufferContext.cxx
  vertexDataBlock.cxx
  vertexDataBook.cxx
  vertexDataBuffer.cxx
  vertexDataPage.cxx
  vertexDataSaveFile.cxx
  vertexSlider.cxx
  vertexTransform.cxx
  videoTexture.cxx
)

set(P3GOBJ_IGATEEXT
  internalName_ext.cxx
  internalName_ext.h
  geomVertexArrayData_ext.cxx
  geomVertexArrayData_ext.h
  texture_ext.cxx
  texture_ext.h
  textureCollection_ext.cxx
  textureCollection_ext.h
)

composite_sources(p3gobj P3GOBJ_SOURCES)
add_library(p3gobj ${P3GOBJ_HEADERS} ${P3GOBJ_SOURCES})
target_link_libraries(p3gobj p3gsgbase p3pnmimage
  ${_ZLIB_LIBRARIES} ${_SQUISH_LIBRARY} ${_CG_LIBRARIES})
target_interrogate(p3gobj ALL EXTENSIONS ${P3GOBJ_IGATEEXT})

install(TARGETS p3gobj DESTINATION lib)

#begin test_bin_target
  #define TARGET test_gobj
  #define LOCAL_LIBS \
#    p3gobj p3putil

  #define SOURCES \
#    test_gobj.cxx

#end test_bin_target
