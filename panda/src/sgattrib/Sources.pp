#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET sgattrib
  #define LOCAL_LIBS \
    sgraph gobj graph putil pnmimage display mathutil gsgbase linmath

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx $[TARGET]_composite3.cxx 
  
  #define SOURCES \
     alphaTransformAttribute.I alphaTransformAttribute.h \
     alphaTransformProperty.I alphaTransformProperty.h \
     alphaTransformTransition.I alphaTransformTransition.h \
     attribTraverser.h billboardTransition.I \
     billboardTransition.h clipPlaneAttribute.I \
     clipPlaneAttribute.h clipPlaneTransition.I \
     clipPlaneTransition.h colorAttribute.I colorAttribute.h \
     colorBlendAttribute.I colorBlendAttribute.h \
     colorBlendProperty.I colorBlendProperty.h \
     colorBlendTransition.I colorBlendTransition.h \
     colorMaskAttribute.I colorMaskAttribute.h \
     colorMaskProperty.I colorMaskProperty.h \
     colorMaskTransition.I colorMaskTransition.h \
     colorMatrixAttribute.I colorMatrixAttribute.h \
     colorMatrixTransition.I colorMatrixTransition.h \
     colorProperty.I colorProperty.h colorTransition.I \
     colorTransition.h config_sgattrib.h cullFaceAttribute.I \
     cullFaceAttribute.h cullFaceProperty.I cullFaceProperty.h \
     cullFaceTransition.I cullFaceTransition.h decalAttribute.I \
     decalAttribute.h decalTransition.I decalTransition.h \
     depthTestAttribute.I depthTestAttribute.h \
     depthTestProperty.I depthTestProperty.h \
     depthTestTransition.I depthTestTransition.h \
     depthWriteAttribute.I depthWriteAttribute.h \
     depthWriteTransition.I depthWriteTransition.h \
     drawBoundsTransition.I drawBoundsTransition.h fogAttribute.I \
     fogAttribute.h fogTransition.I fogTransition.h \
     linesmoothAttribute.I linesmoothAttribute.h \
     linesmoothTransition.I linesmoothTransition.h \
     materialAttribute.I materialAttribute.h materialTransition.I \
     materialTransition.h pointShapeAttribute.I \
     pointShapeAttribute.h pointShapeProperty.I \
     pointShapeProperty.h pointShapeTransition.I \
     pointShapeTransition.h polygonOffsetAttribute.I \
     polygonOffsetAttribute.h polygonOffsetProperty.I \
     polygonOffsetProperty.h polygonOffsetTransition.I \
     polygonOffsetTransition.h pruneTransition.I \
     pruneTransition.h renderModeAttribute.I \
     renderModeAttribute.h renderModeProperty.I \
     renderModeProperty.h renderModeTransition.I \
     renderModeTransition.h renderRelation.I renderRelation.N \
     renderRelation.h showHideAttribute.I showHideAttribute.h \
     showHideTransition.I showHideTransition.h stencilAttribute.I \
     stencilAttribute.h stencilProperty.I stencilProperty.h \
     stencilTransition.I stencilTransition.h texGenAttribute.I \
     texGenAttribute.h texGenProperty.I texGenProperty.h \
     texGenTransition.I texGenTransition.h texMatrixAttribute.I \
     texMatrixAttribute.h texMatrixTransition.I \
     texMatrixTransition.h textureApplyAttribute.I \
     textureApplyAttribute.h textureApplyProperty.I \
     textureApplyProperty.h textureApplyTransition.I \
     textureApplyTransition.h textureAttribute.I \
     textureAttribute.h textureTransition.I textureTransition.h \
     transformAttribute.I transformAttribute.h \
     transformTransition.I transformTransition.h \
     transparencyAttribute.I transparencyAttribute.h \
     transparencyProperty.I transparencyProperty.h \
     transparencyTransition.I transparencyTransition.h

    #define INCLUDED_SOURCES \
        alphaTransformAttribute.cxx alphaTransformProperty.cxx \
        alphaTransformTransition.cxx attribTraverser.cxx billboardTransition.cxx \
        clipPlaneAttribute.cxx clipPlaneTransition.cxx colorAttribute.cxx \
        colorBlendAttribute.cxx colorBlendProperty.cxx colorBlendTransition.cxx \
        colorMaskAttribute.cxx colorMaskProperty.cxx colorMaskTransition.cxx \
        colorMatrixAttribute.cxx colorMatrixTransition.cxx colorProperty.cxx \
        colorTransition.cxx config_sgattrib.cxx cullFaceAttribute.cxx \
        cullFaceProperty.cxx cullFaceTransition.cxx decalAttribute.cxx \
        decalTransition.cxx depthTestAttribute.cxx depthTestProperty.cxx \
        depthTestTransition.cxx depthWriteAttribute.cxx depthWriteTransition.cxx \
        drawBoundsTransition.cxx fogAttribute.cxx fogTransition.cxx \
        linesmoothAttribute.cxx linesmoothTransition.cxx materialAttribute.cxx \
        materialTransition.cxx pointShapeAttribute.cxx pointShapeProperty.cxx \
        pointShapeTransition.cxx polygonOffsetAttribute.cxx \
        polygonOffsetProperty.cxx polygonOffsetTransition.cxx pruneTransition.cxx \
        renderModeAttribute.cxx renderModeProperty.cxx renderModeTransition.cxx \
        renderRelation.cxx showHideAttribute.cxx showHideTransition.cxx \
        stencilAttribute.cxx stencilProperty.cxx stencilTransition.cxx \
        texGenAttribute.cxx texGenProperty.cxx texGenTransition.cxx \
        texMatrixAttribute.cxx texMatrixTransition.cxx textureApplyAttribute.cxx \
        textureApplyProperty.cxx textureApplyTransition.cxx textureAttribute.cxx \
        textureTransition.cxx transformAttribute.cxx transformTransition.cxx \
        transparencyAttribute.cxx transparencyProperty.cxx \
        transparencyTransition.cxx
        
  #define INSTALL_HEADERS \
    alphaTransformAttribute.I alphaTransformAttribute.h \
    alphaTransformProperty.I alphaTransformProperty.h \
    alphaTransformTransition.I alphaTransformTransition.h \
    attribTraverser.h billboardTransition.I billboardTransition.h \
    clipPlaneAttribute.I clipPlaneAttribute.h clipPlaneTransition.I \
    clipPlaneTransition.h colorAttribute.I colorAttribute.h \
    colorBlendAttribute.I colorBlendAttribute.h colorBlendProperty.I \
    colorBlendProperty.h colorBlendTransition.I colorBlendTransition.h \
    colorMaskAttribute.I colorMaskAttribute.h colorMaskProperty.I \
    colorMaskProperty.h colorMaskTransition.I colorMaskTransition.h \
    colorMatrixAttribute.I colorMatrixAttribute.h \
    colorMatrixTransition.I colorMatrixTransition.h colorProperty.I \
    colorProperty.h colorTransition.I colorTransition.h \
    config_sgattrib.h cullFaceAttribute.I cullFaceAttribute.h \
    cullFaceProperty.I cullFaceProperty.h cullFaceTransition.I \
    cullFaceTransition.h decalAttribute.I decalAttribute.h \
    decalTransition.I decalTransition.h depthTestAttribute.I \
    depthTestAttribute.h depthTestProperty.I depthTestProperty.h \
    depthTestTransition.I depthTestTransition.h depthWriteAttribute.I \
    depthWriteAttribute.h depthWriteTransition.I depthWriteTransition.h \
    drawBoundsTransition.I drawBoundsTransition.h fogAttribute.I \
    fogAttribute.h fogTransition.I fogTransition.h \
    linesmoothAttribute.I linesmoothAttribute.h linesmoothTransition.I \
    linesmoothTransition.h materialAttribute.I materialAttribute.h \
    materialTransition.I materialTransition.h pointShapeAttribute.I \
    pointShapeAttribute.h pointShapeProperty.I pointShapeProperty.h \
    pointShapeTransition.I pointShapeTransition.h \
    polygonOffsetAttribute.I polygonOffsetAttribute.h \
    polygonOffsetProperty.I polygonOffsetProperty.h \
    polygonOffsetTransition.I polygonOffsetTransition.h \
    pruneTransition.I pruneTransition.h renderModeAttribute.I \
    renderModeAttribute.h renderModeProperty.I renderModeProperty.h \
    renderModeTransition.I renderModeTransition.h renderRelation.I \
    renderRelation.h showHideAttribute.I showHideAttribute.h \
    showHideNameClass.h showHideTransition.I showHideTransition.h \
    stencilAttribute.I stencilAttribute.h stencilProperty.I \
    stencilProperty.h stencilTransition.I stencilTransition.h \
    texGenAttribute.I texGenAttribute.h texGenProperty.I \
    texGenProperty.h texGenTransition.I texGenTransition.h \
    texMatrixAttribute.I texMatrixAttribute.h texMatrixTransition.I \
    texMatrixTransition.h textureApplyAttribute.I \
    textureApplyAttribute.h textureApplyProperty.I \
    textureApplyProperty.h textureApplyTransition.I \
    textureApplyTransition.h textureAttribute.I textureAttribute.h \
    textureTransition.I textureTransition.h transformAttribute.I \
    transformAttribute.h transformTransition.I transformTransition.h \
    transparencyAttribute.I transparencyAttribute.h \
    transparencyProperty.I transparencyProperty.h \
    transparencyTransition.I transparencyTransition.h

  #define IGATESCAN all
  
//  #define PRECOMPILED_HEADER sgattrib_headers.h
// 
//  #define IGATESCAN \
//    renderRelation.cxx textureTransition.cxx materialTransition.cxx \
//    clipPlaneTransition.cxx linesmoothTransition.cxx transformTransition.cxx \
//    texMatrixTransition.cxx colorTransition.cxx billboardTransition.cxx \
//    depthWriteTransition.cxx decalTransition.cxx \
//    showHideTransition.cxx showHideAttribute.cxx pruneTransition.cxx \
//    fogTransition.cxx transparencyTransition.cxx polygonOffsetTransition.cxx \
//    colorMatrixTransition.cxx alphaTransformTransition.cxx 

#end lib_target

#begin test_bin_target
  #define TARGET test_attrib
  #define LOCAL_LIBS \
    sgraph gobj graph putil sgattrib pnmimage display mathutil gsgbase \
    linmath

  #define SOURCES \
    test_attrib.cxx

#end test_bin_target

