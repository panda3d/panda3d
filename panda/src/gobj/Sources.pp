#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET gobj
  #define LOCAL_LIBS \
    linmath mathutil pnmimage gsgbase graph putil

  #define SOURCES \
    LOD.I LOD.cxx LOD.h config_gobj.cxx config_gobj.h drawable.cxx \
    drawable.h fog.I fog.cxx fog.h geom.I geom.N geom.cxx geom.h \
    geomLine.cxx geomLine.h geomLinestrip.cxx geomLinestrip.h \
    geomPoint.cxx geomPoint.h geomPolygon.cxx geomPolygon.h \
    geomQuad.cxx geomQuad.h geomSphere.cxx geomSphere.h geomSprite.I \
    geomSprite.cxx geomSprite.h geomTri.cxx geomTri.h geomTrifan.cxx \
    geomTrifan.h geomTristrip.cxx geomTristrip.h \
    imageBuffer.I imageBuffer.cxx \
    imageBuffer.h material.I material.cxx material.h \
    materialPool.I materialPool.h materialPool.cxx \
    orthoProjection.I \
    orthoProjection.cxx orthoProjection.h perspectiveProjection.I \
    perspectiveProjection.cxx perspectiveProjection.h pixelBuffer.I \
    pixelBuffer.N pixelBuffer.cxx pixelBuffer.h projection.cxx \
    projection.h texture.I texture.N texture.cxx texture.h \
    texturePool.I texturePool.cxx texturePool.h

  #define INSTALL_HEADERS \
    LOD.I LOD.h config_gobj.h \
    drawable.h fog.I fog.h geom.I geom.h geomLine.h \
    geomLinestrip.h geomPoint.h geomPolygon.h geomQuad.h geomSphere.h \
    geomSprite.I geomSprite.h geomTri.h geomTrifan.h geomTristrip.h \
    geomprimitives.h imageBuffer.I imageBuffer.h material.I material.h \
    materialPool.I materialPool.h \
    orthoProjection.I orthoProjection.h perspectiveProjection.I \
    perspectiveProjection.h pixelBuffer.I pixelBuffer.h projection.h \
    texture.I texture.h texturePool.I texturePool.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_gobj
  #define LOCAL_LIBS \
    gobj putil

  #define SOURCES \
    test_gobj.cxx

#end test_bin_target

