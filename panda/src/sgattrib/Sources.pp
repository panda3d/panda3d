#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgattrib
  #define LOCAL_LIBS \
    sgraph gobj graph putil pnmimage display mathutil gsgbase linmath

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx $[TARGET]_composite3.cxx 
  
  #define SOURCES \
     alphaTransformProperty.I alphaTransformProperty.h \
     alphaTransformTransition.I alphaTransformTransition.h \
     attribTraverser.h billboardTransition.I \
     billboardTransition.h clipPlaneTransition.I \
     clipPlaneTransition.h \
     colorBlendProperty.I colorBlendProperty.h \
     colorBlendTransition.I colorBlendTransition.h \
     colorMaskProperty.I colorMaskProperty.h \
     colorMaskTransition.I colorMaskTransition.h \
     colorMatrixTransition.I colorMatrixTransition.h \
     colorProperty.I colorProperty.h colorTransition.I \
     colorTransition.h config_sgattrib.h \
     cullFaceProperty.I cullFaceProperty.h \
     cullFaceTransition.I cullFaceTransition.h \
     decalTransition.I decalTransition.h \
     depthTestProperty.I depthTestProperty.h \
     depthTestTransition.I depthTestTransition.h \
     depthWriteTransition.I depthWriteTransition.h \
     drawBoundsTransition.I drawBoundsTransition.h \
     fogTransition.I fogTransition.h \
     linesmoothTransition.I linesmoothTransition.h \
     materialTransition.I \
     materialTransition.h pointShapeProperty.I \
     pointShapeProperty.h pointShapeTransition.I \
     pointShapeTransition.h polygonOffsetProperty.I \
     polygonOffsetProperty.h polygonOffsetTransition.I \
     polygonOffsetTransition.h pruneTransition.I \
     pruneTransition.h renderModeProperty.I \
     renderModeProperty.h renderModeTransition.I \
     renderModeTransition.h renderRelation.I renderRelation.N \
     renderRelation.h stencilProperty.I stencilProperty.h \
     stencilTransition.I stencilTransition.h \
     texGenProperty.I texGenProperty.h \
     texGenTransition.I texGenTransition.h texMatrixTransition.I \
     texMatrixTransition.h textureApplyProperty.I \
     textureApplyProperty.h textureApplyTransition.I \
     textureApplyTransition.h textureTransition.I textureTransition.h \
     transformTransition.I transformTransition.h \
     transparencyProperty.I transparencyProperty.h \
     transparencyTransition.I transparencyTransition.h

    #define INCLUDED_SOURCES \
        alphaTransformProperty.cxx \
        alphaTransformTransition.cxx attribTraverser.cxx billboardTransition.cxx \
        clipPlaneTransition.cxx  \
        colorBlendProperty.cxx colorBlendTransition.cxx \
        colorMaskProperty.cxx colorMaskTransition.cxx \
        colorMatrixTransition.cxx colorProperty.cxx \
        colorTransition.cxx config_sgattrib.cxx \
        cullFaceProperty.cxx cullFaceTransition.cxx \
        decalTransition.cxx depthTestProperty.cxx \
        depthTestTransition.cxx depthWriteTransition.cxx \
        drawBoundsTransition.cxx fogTransition.cxx \
        linesmoothTransition.cxx \
        materialTransition.cxx pointShapeProperty.cxx \
        pointShapeTransition.cxx \
        polygonOffsetProperty.cxx polygonOffsetTransition.cxx pruneTransition.cxx \
        renderModeProperty.cxx renderModeTransition.cxx \
        renderRelation.cxx \
        stencilProperty.cxx stencilTransition.cxx \
        texGenProperty.cxx texGenTransition.cxx \
        texMatrixTransition.cxx \
        textureApplyProperty.cxx textureApplyTransition.cxx \
        textureTransition.cxx transformTransition.cxx \
        transparencyProperty.cxx \
        transparencyTransition.cxx
        
  #define INSTALL_HEADERS \
    alphaTransformProperty.I alphaTransformProperty.h \
    alphaTransformTransition.I alphaTransformTransition.h \
    attribTraverser.h billboardTransition.I billboardTransition.h \
    clipPlaneTransition.I \
    clipPlaneTransition.h \
    colorBlendProperty.I \
    colorBlendProperty.h colorBlendTransition.I colorBlendTransition.h \
    colorMaskProperty.I \
    colorMaskProperty.h colorMaskTransition.I colorMaskTransition.h \
    colorMatrixTransition.I colorMatrixTransition.h colorProperty.I \
    colorProperty.h colorTransition.I colorTransition.h \
    config_sgattrib.h \
    cullFaceProperty.I cullFaceProperty.h cullFaceTransition.I \
    cullFaceTransition.h \
    decalTransition.I decalTransition.h \
    depthTestProperty.I depthTestProperty.h \
    depthTestTransition.I depthTestTransition.h \
    depthWriteTransition.I depthWriteTransition.h \
    drawBoundsTransition.I drawBoundsTransition.h \
    fogTransition.I fogTransition.h \
    linesmoothTransition.I \
    linesmoothTransition.h \
    materialTransition.I materialTransition.h \
    pointShapeProperty.I pointShapeProperty.h \
    pointShapeTransition.I pointShapeTransition.h \
    polygonOffsetProperty.I polygonOffsetProperty.h \
    polygonOffsetTransition.I polygonOffsetTransition.h \
    pruneTransition.I pruneTransition.h \
    renderModeProperty.I renderModeProperty.h \
    renderModeTransition.I renderModeTransition.h renderRelation.I \
    renderRelation.h \
    stencilProperty.I \
    stencilProperty.h stencilTransition.I stencilTransition.h \
    texGenProperty.I \
    texGenProperty.h texGenTransition.I texGenTransition.h \
    texMatrixTransition.I \
    texMatrixTransition.h \
    textureApplyProperty.I \
    textureApplyProperty.h textureApplyTransition.I \
    textureApplyTransition.h \
    textureTransition.I textureTransition.h \
    transformTransition.I transformTransition.h \
    transparencyProperty.I transparencyProperty.h \
    transparencyTransition.I transparencyTransition.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_attrib
  #define LOCAL_LIBS \
    sgraph gobj graph putil sgattrib pnmimage display mathutil gsgbase \
    linmath

  #define SOURCES \
    test_attrib.cxx

#end test_bin_target

