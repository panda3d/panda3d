#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define OSX_SYS_LIBS mx

#begin lib_target
  #define TARGET gobj
  #define LOCAL_LIBS \
    pstatclient event linmath mathutil pnmimage gsgbase putil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
    boundedObject.I boundedObject.h \
    config_gobj.h \
    drawable.h \
    geom.h geom.I \
    geomContext.I geomContext.h \
    geomEnums.h \
    geomMunger.h geomMunger.I \
    geomPrimitive.h geomPrimitive.I \
    geomTriangles.h \
    geomTristrips.h \
    geomTrifans.h \
    geomLines.h \
    geomLinestrips.h \
    geomPoints.h \
    geomVertexArrayData.h geomVertexArrayData.I \
    geomVertexArrayFormat.h geomVertexArrayFormat.I \
    geomCacheEntry.h geomCacheEntry.I \
    geomCacheManager.h geomCacheManager.I \
    geomVertexAnimationSpec.h geomVertexAnimationSpec.I \
    geomVertexData.h geomVertexData.I \
    geomVertexColumn.h geomVertexColumn.I \
    geomVertexFormat.h geomVertexFormat.I \
    geomVertexReader.h geomVertexReader.I \
    geomVertexRewriter.h geomVertexRewriter.I \
    geomVertexWriter.h geomVertexWriter.I \
    indexBufferContext.I indexBufferContext.h \
    internalName.I internalName.h \
    material.I material.h materialPool.I materialPool.h  \
    matrixLens.I matrixLens.h \
    orthographicLens.I orthographicLens.h perspectiveLens.I  \
    perspectiveLens.h \
    preparedGraphicsObjects.I preparedGraphicsObjects.h \
    lens.h lens.I \
    savedContext.I savedContext.h \
    shader.I shader.h \
    sliderTable.I sliderTable.h \
    texture.I texture.h \
    textureContext.I textureContext.h \
    texturePool.I texturePool.h \
    textureStage.I textureStage.h \
    transformBlend.I transformBlend.h \
    transformBlendTable.I transformBlendTable.h \
    transformTable.I transformTable.h \
    userVertexSlider.I userVertexSlider.h \
    userVertexTransform.I userVertexTransform.h \
    vertexBufferContext.I vertexBufferContext.h \
    vertexSlider.I vertexSlider.h \
    vertexTransform.I vertexTransform.h
    
  #define INCLUDED_SOURCES \
    boundedObject.cxx \
    config_gobj.cxx \
    drawable.cxx \
    geomContext.cxx \
    geom.cxx \
    geomEnums.cxx \
    geomMunger.cxx \
    geomPrimitive.cxx \
    geomTriangles.cxx \
    geomTristrips.cxx \
    geomTrifans.cxx \
    geomLines.cxx \
    geomLinestrips.cxx \
    geomPoints.cxx \
    geomVertexArrayData.cxx \
    geomVertexArrayFormat.cxx \
    geomCacheEntry.cxx \
    geomCacheManager.cxx \
    geomVertexAnimationSpec.cxx \
    geomVertexData.cxx \
    geomVertexColumn.cxx \
    geomVertexFormat.cxx \
    geomVertexReader.cxx \
    geomVertexRewriter.cxx \
    geomVertexWriter.cxx \
    indexBufferContext.cxx \
    material.cxx  \
    internalName.cxx \
    materialPool.cxx matrixLens.cxx orthographicLens.cxx  \
    perspectiveLens.cxx \
    preparedGraphicsObjects.cxx \
    lens.cxx  \
    savedContext.cxx \
    shader.cxx \
    sliderTable.cxx \
    texture.cxx textureContext.cxx texturePool.cxx \
    textureStage.cxx \
    transformBlend.cxx \
    transformBlendTable.cxx \
    transformTable.cxx \
    userVertexSlider.cxx \
    userVertexTransform.cxx \
    vertexBufferContext.cxx \
    vertexSlider.cxx \
    vertexTransform.cxx

  #define INSTALL_HEADERS \
    boundedObject.I boundedObject.h \
    config_gobj.h \
    drawable.h geom.I geom.h \
    textureContext.I textureContext.h \
    geom.h geom.I \
    geomContext.I geomContext.h \
    geomEnums.h \
    geomMunger.h geomMunger.I \
    geomPrimitive.h geomPrimitive.I \
    geomTriangles.h \
    geomTristrips.h \
    geomTrifans.h \
    geomLines.h \
    geomLinestrips.h \
    geomPoints.h \
    geomVertexArrayData.h geomVertexArrayData.I \
    geomVertexArrayFormat.h geomVertexArrayFormat.I \
    geomCacheEntry.h geomCacheEntry.I \
    geomCacheManager.h geomCacheManager.I \
    geomVertexAnimationSpec.h geomVertexAnimationSpec.I \
    geomVertexData.h geomVertexData.I \
    geomVertexColumn.h geomVertexColumn.I \
    geomVertexFormat.h geomVertexFormat.I \
    geomVertexReader.h geomVertexReader.I \
    geomVertexRewriter.h geomVertexRewriter.I \
    geomVertexWriter.h geomVertexWriter.I \
    indexBufferContext.I indexBufferContext.h \
    internalName.I internalName.h \
    material.I material.h \
    materialPool.I materialPool.h matrixLens.I matrixLens.h \
    orthographicLens.I orthographicLens.h perspectiveLens.I \
    perspectiveLens.h \
    preparedGraphicsObjects.I preparedGraphicsObjects.h \
    lens.h lens.I \
    savedContext.I savedContext.h \
    shader.h shader.I \
    shaderContext.h shaderContext.I \
    sliderTable.I sliderTable.h \
    texture.I texture.h \
    textureContext.I textureContext.h \
    texturePool.I texturePool.h \
    textureStage.I textureStage.h \
    transformBlend.I transformBlend.h \
    transformBlendTable.I transformBlendTable.h \
    transformTable.I transformTable.h \
    userVertexSlider.I userVertexSlider.h \
    userVertexTransform.I userVertexTransform.h \
    vertexBufferContext.I vertexBufferContext.h \
    vertexSlider.I vertexSlider.h \
    vertexTransform.I vertexTransform.h


  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_gobj
  #define LOCAL_LIBS \
    gobj putil

  #define SOURCES \
    test_gobj.cxx

#end test_bin_target

