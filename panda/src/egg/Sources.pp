#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define YACC_PREFIX eggyy
#define LFLAGS -i

#begin lib_target
  #define TARGET egg
  #define LOCAL_LIBS \
    linmath putil

  #define SOURCES \
    config_egg.cxx config_egg.h eggAnimData.I eggAnimData.cxx eggAnimData.h \
    eggAttributes.I eggAttributes.cxx eggAttributes.h eggBin.cxx \
    eggBin.h eggBinMaker.cxx eggBinMaker.h eggComment.I eggComment.cxx \
    eggComment.h eggCoordinateSystem.I eggCoordinateSystem.cxx \
    eggCoordinateSystem.h eggCurve.I eggCurve.cxx eggCurve.h eggData.I \
    eggData.cxx eggData.h eggExternalReference.I \
    eggExternalReference.cxx eggExternalReference.h eggFilenameNode.I \
    eggFilenameNode.cxx eggFilenameNode.h eggGroup.I eggGroup.cxx \
    eggGroup.h eggGroupNode.I eggGroupNode.cxx eggGroupNode.h \
    eggGroupUniquifier.h eggGroupUniquifier.cxx \
    eggMaterial.I eggMaterial.cxx eggMaterial.h \
    eggMaterialCollection.I eggMaterialCollection.cxx \
    eggMaterialCollection.h \
    eggMiscFuncs.I \
    eggMiscFuncs.cxx eggMiscFuncs.h eggNamedObject.I eggNamedObject.cxx \
    eggNamedObject.h eggNameUniquifier.cxx eggNameUniquifier.h \
    eggNode.I eggNode.cxx eggNode.h eggNurbsCurve.I \
    eggNurbsCurve.cxx eggNurbsCurve.h eggNurbsSurface.I \
    eggNurbsSurface.cxx eggNurbsSurface.h eggObject.I eggObject.cxx \
    eggObject.h eggParameters.cxx eggParameters.h eggPoint.I \
    eggPoint.cxx eggPoint.h eggPolygon.I eggPolygon.cxx eggPolygon.h \
    eggPoolUniquifier.cxx eggPoolUniquifier.h \
    eggPrimitive.I eggPrimitive.cxx eggPrimitive.h \
    eggRenderMode.I eggRenderMode.cxx \
    eggRenderMode.h eggSAnimData.I \
    eggSAnimData.cxx eggSAnimData.h eggSurface.I eggSurface.cxx \
    eggSurface.h eggSwitchCondition.cxx eggSwitchCondition.h eggTable.I \
    eggTable.cxx eggTable.h eggTexture.I eggTexture.cxx eggTexture.h \
    eggTextureCollection.I eggTextureCollection.cxx \
    eggTextureCollection.h eggUtilities.I eggUtilities.cxx \
    eggUtilities.h eggVertex.I eggVertex.cxx eggVertex.h \
    eggVertexPool.I eggVertexPool.cxx eggVertexPool.h eggXfmAnimData.I \
    eggXfmAnimData.cxx eggXfmAnimData.h eggXfmSAnim.I eggXfmSAnim.cxx \
    eggXfmSAnim.h parserDefs.h parser.yxx lexerDefs.h lexer.lxx \
    pt_EggVertex.cxx pt_EggVertex.h \
    vector_PT_EggVertex.cxx vector_PT_EggVertex.h

  #define INSTALL_HEADERS \
    eggAnimData.I eggAnimData.h \
    eggAttributes.I eggAttributes.h eggBin.h eggBinMaker.h eggComment.I \
    eggComment.h eggCoordinateSystem.I eggCoordinateSystem.h eggCurve.I \
    eggCurve.h eggData.I eggData.h eggExternalReference.I \
    eggExternalReference.h eggFilenameNode.I eggFilenameNode.h \
    eggGroup.I eggGroup.h eggGroupNode.I eggGroupNode.h \
    eggGroupUniquifier.h eggMaterial.I \
    eggMaterial.h eggMaterialCollection.I eggMaterialCollection.h \
    eggMorph.I eggMorph.h eggMorphList.I eggMorphList.h \
    eggNamedObject.I eggNamedObject.h eggNameUniquifier.h eggNode.I eggNode.h \
    eggNurbsCurve.I eggNurbsCurve.h eggNurbsSurface.I eggNurbsSurface.h \
    eggObject.I eggObject.h eggParameters.h eggPoint.I eggPoint.h \
    eggPolygon.I eggPolygon.h eggPoolUniquifier.h \
    eggPrimitive.I eggPrimitive.h eggRenderMode.I eggRenderMode.h \
    eggSAnimData.I eggSAnimData.h eggSurface.I eggSurface.h \
    eggSwitchCondition.h eggTable.I eggTable.h eggTexture.I \
    eggTexture.h eggTextureCollection.I eggTextureCollection.h \
    eggUtilities.I eggUtilities.h eggVertex.I eggVertex.h \
    eggVertexPool.I eggVertexPool.h eggXfmAnimData.I eggXfmAnimData.h \
    eggXfmSAnim.I eggXfmSAnim.h \
    pt_EggVertex.h vector_PT_EggVertex.h


#end lib_target

#begin test_bin_target
  #define TARGET test_egg
  #define LOCAL_LIBS \
    egg putil mathutil

  #define SOURCES \
    test_egg.cxx

#end test_bin_target

