#define BUILD_DIRECTORY $[HAVE_EGG]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define YACC_PREFIX eggyy
#define FLEXFLAGS -i
#define USE_PACKAGES zlib

#begin lib_target
  #define TARGET p3egg
  #define LOCAL_LIBS \
    p3mathutil p3linmath p3putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
     config_egg.h eggAnimData.I eggAnimData.h \
     eggAnimPreload.I eggAnimPreload.h \
     eggAttributes.I  \
     eggAttributes.h eggBin.h eggBinMaker.h eggComment.I  \
     eggComment.h \
     eggCompositePrimitive.I eggCompositePrimitive.h \
     eggCoordinateSystem.I eggCoordinateSystem.h  \
     eggCurve.I eggCurve.h eggData.I eggData.h  \
     eggExternalReference.I eggExternalReference.h  \
     eggFilenameNode.I eggFilenameNode.h eggGroup.I eggGroup.h  \
     eggGroupNode_ext.cxx eggGroupNode_ext.h \
     eggGroupNode.I eggGroupNode.h eggGroupUniquifier.h  \
     eggLine.I eggLine.h \
     eggMaterial.I eggMaterial.h eggMaterialCollection.I  \
     eggMaterialCollection.h \
     eggMesher.h eggMesher.I \
     eggMesherEdge.h eggMesherEdge.I \
     eggMesherFanMaker.h eggMesherFanMaker.I \
     eggMesherStrip.h eggMesherStrip.I \
     eggMiscFuncs.I eggMiscFuncs.h  \
     eggMorph.I eggMorph.h eggMorphList.I eggMorphList.h  \
     eggNamedObject.I eggNamedObject.h eggNameUniquifier.h  \
     eggNode.I eggNode.h eggNurbsCurve.I eggNurbsCurve.h  \
     eggNurbsSurface.I eggNurbsSurface.h eggObject.I eggObject.h  \
     eggParameters.h \
     eggPatch.I eggPatch.h \
     eggPoint.I eggPoint.h eggPolygon.I  \
     eggPolygon.h eggPolysetMaker.h eggPoolUniquifier.h \
     eggPrimitive.I eggPrimitive.h \
     eggRenderMode.I eggRenderMode.h  \
     eggSAnimData.I eggSAnimData.h eggSurface.I eggSurface.h  \
     eggSwitchCondition.h eggTable.I eggTable.h eggTexture.I  \
     eggTexture.h eggTextureCollection.I eggTextureCollection.h  \
     eggTriangleFan.I eggTriangleFan.h \
     eggTriangleStrip.I eggTriangleStrip.h \
     eggTransform.I eggTransform.h \
     eggUserData.I eggUserData.h \
     eggUtilities.I eggUtilities.h \
     eggVertex.I eggVertex.h \
     eggVertexAux.I eggVertexAux.h \
     eggVertexPool.I eggVertexPool.h \
     eggVertexUV.I eggVertexUV.h \
     eggXfmAnimData.I  \
     eggXfmAnimData.h eggXfmSAnim.I eggXfmSAnim.h parserDefs.h  \
     parser.yxx lexerDefs.h lexer.lxx pt_EggMaterial.h  \
     vector_PT_EggMaterial.h pt_EggTexture.h  \
     vector_PT_EggTexture.h pt_EggVertex.h vector_PT_EggVertex.h

  #define INCLUDED_SOURCES \
     config_egg.cxx eggAnimData.cxx \
     eggAnimPreload.cxx \
     eggAttributes.cxx eggBin.cxx  \
     eggBinMaker.cxx eggComment.cxx \
     eggCompositePrimitive.cxx \
     eggCoordinateSystem.cxx  \
     eggCurve.cxx eggData.cxx eggExternalReference.cxx  \
     eggFilenameNode.cxx eggGroup.cxx  \
     eggGroupNode.cxx  \
     eggGroupUniquifier.cxx eggLine.cxx eggMaterial.cxx  \
     eggMaterialCollection.cxx \
     eggMesher.cxx \
     eggMesherEdge.cxx \
     eggMesherFanMaker.cxx \
     eggMesherStrip.cxx \
     eggMiscFuncs.cxx eggMorphList.cxx  \
     eggNamedObject.cxx eggNameUniquifier.cxx eggNode.cxx  \
     eggNurbsCurve.cxx eggNurbsSurface.cxx eggObject.cxx  \
     eggParameters.cxx \
     eggPatch.cxx \
     eggPoint.cxx eggPolygon.cxx eggPolysetMaker.cxx  \
     eggPoolUniquifier.cxx eggPrimitive.cxx eggRenderMode.cxx  \
     eggSAnimData.cxx eggSurface.cxx eggSwitchCondition.cxx  \
     eggTable.cxx eggTexture.cxx eggTextureCollection.cxx  \
     eggTransform.cxx \
     eggTriangleFan.cxx \
     eggTriangleStrip.cxx \
     eggUserData.cxx \
     eggUtilities.cxx eggVertex.cxx \
     eggVertexAux.cxx \
     eggVertexPool.cxx eggVertexUV.cxx \
     eggXfmAnimData.cxx eggXfmSAnim.cxx pt_EggMaterial.cxx  \
     vector_PT_EggMaterial.cxx pt_EggTexture.cxx  \
     vector_PT_EggTexture.cxx pt_EggVertex.cxx  \
     vector_PT_EggVertex.cxx 

  #define INSTALL_HEADERS \
    eggAnimData.I eggAnimData.h \
    eggAnimPreload.I eggAnimPreload.h \
    eggAttributes.I eggAttributes.h eggBin.h eggBinMaker.h eggComment.I \
    eggComment.h \
    eggCompositePrimitive.I eggCompositePrimitive.h \
    eggCoordinateSystem.I eggCoordinateSystem.h eggCurve.I \
    eggCurve.h eggData.I eggData.h eggExternalReference.I \
    eggExternalReference.h eggFilenameNode.I eggFilenameNode.h \
    eggGroup.I eggGroup.h \
    eggGroupNode_ext.h eggGroupNode.I eggGroupNode.h \
    eggGroupUniquifier.h \
    eggLine.I eggLine.h \
    eggMaterial.I \
    eggMaterial.h eggMaterialCollection.I eggMaterialCollection.h \
    eggMorph.I eggMorph.h eggMorphList.I eggMorphList.h \
    eggNamedObject.I eggNamedObject.h eggNameUniquifier.h eggNode.I eggNode.h \
    eggNurbsCurve.I eggNurbsCurve.h eggNurbsSurface.I eggNurbsSurface.h \
    eggObject.I eggObject.h eggParameters.h \
    eggPatch.I eggPatch.h \
    eggPoint.I eggPoint.h \
    eggPolygon.I eggPolygon.h eggPolysetMaker.h eggPoolUniquifier.h \
    eggPrimitive.I eggPrimitive.h eggRenderMode.I eggRenderMode.h \
    eggSAnimData.I eggSAnimData.h eggSurface.I eggSurface.h \
    eggSwitchCondition.h eggTable.I eggTable.h eggTexture.I \
    eggTexture.h eggTextureCollection.I eggTextureCollection.h \
    eggTransform.I eggTransform.h \
    eggTriangleFan.I eggTriangleFan.h \
    eggTriangleStrip.I eggTriangleStrip.h \
    eggUserData.I eggUserData.h \
    eggUtilities.I eggUtilities.h \
    eggVertex.I eggVertex.h \
    eggVertexAux.I eggVertexAux.h \
    eggVertexPool.I eggVertexPool.h \
    eggVertexUV.I eggVertexUV.h \
    eggXfmAnimData.I eggXfmAnimData.h \
    eggXfmSAnim.I eggXfmSAnim.h \
    pt_EggMaterial.h vector_PT_EggMaterial.h \
    pt_EggTexture.h vector_PT_EggTexture.h \
    pt_EggVertex.h vector_PT_EggVertex.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_egg
  #define LOCAL_LIBS \
    p3egg p3putil p3mathutil

  #define SOURCES \
    test_egg.cxx

#end test_bin_target

