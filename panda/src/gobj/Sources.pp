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
    geom.I geom.h \
    geomContext.I geomContext.h \
    geomLine.h geomLinestrip.h geomPoint.h geomPolygon.h  \
    geomQuad.h geomSphere.h geomSprite.I geomSprite.h geomTri.h  \
    geomTrifan.h geomTristrip.h  \
    qpgeom.h qpgeom.I \
    qpgeomMunger.h qpgeomMunger.I \
    qpgeomPrimitive.h qpgeomPrimitive.I \
    qpgeomTriangles.h \
    qpgeomTristrips.h \
    qpgeomTrifans.h \
    qpgeomLines.h \
    qpgeomLinestrips.h \
    qpgeomPoints.h \
    qpgeomUsageHint.h \
    qpgeomVertexArrayData.h qpgeomVertexArrayData.I \
    qpgeomVertexArrayFormat.h qpgeomVertexArrayFormat.I \
    qpgeomCacheEntry.h qpgeomCacheEntry.I \
    qpgeomCacheManager.h qpgeomCacheManager.I \
    qpgeomVertexData.h qpgeomVertexData.I \
    qpgeomVertexDataType.h qpgeomVertexDataType.I \
    qpgeomVertexFormat.h qpgeomVertexFormat.I \
    qpgeomVertexReader.h qpgeomVertexReader.I \
    qpgeomVertexWriter.h qpgeomVertexWriter.I \
    indexBufferContext.I indexBufferContext.h \
    internalName.I internalName.h \
    material.I material.h materialPool.I materialPool.h  \
    matrixLens.I matrixLens.h \
    orthographicLens.I orthographicLens.h perspectiveLens.I  \
    perspectiveLens.h \
    preparedGraphicsObjects.I preparedGraphicsObjects.h \
    lens.h lens.I \
    savedContext.I savedContext.h \
    texture.I texture.h \
    textureContext.I textureContext.h \
    texturePool.I texturePool.h \
    textureStage.I textureStage.h \
    transformBlend.I transformBlend.h \
    transformBlendPalette.I transformBlendPalette.h \
    transformPalette.I transformPalette.h \
    userVertexTransform.I userVertexTransform.h \
    vertexBufferContext.I vertexBufferContext.h \
    vertexTransform.I vertexTransform.h
    
  #define INCLUDED_SOURCES \
    boundedObject.cxx \
    config_gobj.cxx \
    drawable.cxx geom.cxx  \
    geomContext.cxx \
    geomLine.cxx geomLinestrip.cxx geomPoint.cxx geomPolygon.cxx  \
    geomQuad.cxx geomSphere.cxx geomSprite.cxx geomTri.cxx  \
    geomTrifan.cxx geomTristrip.cxx \
    qpgeom.cxx \
    qpgeomMunger.cxx \
    qpgeomPrimitive.cxx \
    qpgeomTriangles.cxx \
    qpgeomTristrips.cxx \
    qpgeomTrifans.cxx \
    qpgeomLines.cxx \
    qpgeomLinestrips.cxx \
    qpgeomPoints.cxx \
    qpgeomVertexArrayData.cxx \
    qpgeomVertexArrayFormat.cxx \
    qpgeomCacheEntry.cxx \
    qpgeomCacheManager.cxx \
    qpgeomVertexData.cxx \
    qpgeomVertexDataType.cxx \
    qpgeomVertexFormat.cxx \
    qpgeomVertexReader.cxx \
    qpgeomVertexWriter.cxx \
    indexBufferContext.cxx \
    material.cxx  \
    internalName.cxx \
    materialPool.cxx matrixLens.cxx orthographicLens.cxx  \
    perspectiveLens.cxx \
    preparedGraphicsObjects.cxx \
    lens.cxx  \
    savedContext.cxx texture.cxx textureContext.cxx texturePool.cxx \
    textureStage.cxx \
    transformBlend.cxx \
    transformBlendPalette.cxx \
    transformPalette.cxx \
    userVertexTransform.cxx \
    vertexBufferContext.cxx \
    vertexTransform.cxx

  #define INSTALL_HEADERS \
    boundedObject.I boundedObject.h \
    config_gobj.h \
    drawable.h geom.I geom.h \
    textureContext.I textureContext.h \
    geomLine.h \
    geomLinestrip.h geomPoint.h geomPolygon.h geomQuad.h geomSphere.h \
    geomSprite.I geomSprite.h geomTri.h geomTrifan.h geomTristrip.h \
    geomprimitives.h \
    qpgeom.h qpgeom.I \
    qpgeomMunger.h qpgeomMunger.I \
    qpgeomPrimitive.h qpgeomPrimitive.I \
    qpgeomTriangles.h \
    qpgeomTristrips.h \
    qpgeomTrifans.h \
    qpgeomLines.h \
    qpgeomLinestrips.h \
    qpgeomPoints.h \
    qpgeomUsageHint.h \
    qpgeomVertexArrayData.h qpgeomVertexArrayData.I \
    qpgeomVertexArrayFormat.h qpgeomVertexArrayFormat.I \
    qpgeomCacheEntry.h qpgeomCacheEntry.I \
    qpgeomCacheManager.h qpgeomCacheManager.I \
    qpgeomVertexData.h qpgeomVertexData.I \
    qpgeomVertexDataType.h qpgeomVertexDataType.I \
    qpgeomVertexFormat.h qpgeomVertexFormat.I \
    qpgeomVertexReader.h qpgeomVertexReader.I \
    qpgeomVertexWriter.h qpgeomVertexWriter.I \
    indexBufferContext.I indexBufferContext.h \
    internalName.I internalName.h \
    material.I material.h \
    materialPool.I materialPool.h matrixLens.I matrixLens.h \
    orthographicLens.I orthographicLens.h perspectiveLens.I \
    perspectiveLens.h \
    preparedGraphicsObjects.I preparedGraphicsObjects.h \
    lens.h lens.I \
    savedContext.I savedContext.h \
    texture.I texture.h \
    textureContext.I textureContext.h \
    texturePool.I texturePool.h \
    textureStage.I textureStage.h \
    transformBlend.I transformBlend.h \
    transformBlendPalette.I transformBlendPalette.h \
    transformPalette.I transformPalette.h \
    userVertexTransform.I userVertexTransform.h \
    vertexBufferContext.I vertexBufferContext.h \
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

