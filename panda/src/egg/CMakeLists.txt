if(HAVE_EGG)
  add_bison_target(parser.cxx parser.yxx DEFINES parser.h PREFIX eggyy)
  add_flex_target(lexer.cxx lexer.lxx DEFINES lexer.h PREFIX eggyy)

  set(P3EGG_HEADERS
    config_egg.h eggAnimData.I eggAnimData.h
    eggAnimPreload.I eggAnimPreload.h
    eggAttributes.Ig
    eggAttributes.h eggBin.h eggBinMaker.h eggComment.Ig
    eggComment.h
    eggCompositePrimitive.I eggCompositePrimitive.h
    eggCoordinateSystem.I eggCoordinateSystem.hg
    eggCurve.I eggCurve.h eggData.I eggData.hg
    eggExternalReference.I eggExternalReference.hg
    eggFilenameNode.I eggFilenameNode.h eggGroup.I eggGroup.hg
    eggGroupNode.I eggGroupNode.h eggGroupUniquifier.hg
    eggLine.I eggLine.h
    eggMaterial.I eggMaterial.h eggMaterialCollection.Ig
    eggMaterialCollection.h
    eggMesher.h eggMesher.I
    eggMesherEdge.h eggMesherEdge.I
    eggMesherFanMaker.h eggMesherFanMaker.I
    eggMesherStrip.h eggMesherStrip.I
    eggMiscFuncs.I eggMiscFuncs.hg
    eggMorph.I eggMorph.h eggMorphList.I eggMorphList.hg
    eggNamedObject.I eggNamedObject.h eggNameUniquifier.hg
    eggNode.I eggNode.h eggNurbsCurve.I eggNurbsCurve.hg
    eggNurbsSurface.I eggNurbsSurface.h eggObject.I eggObject.hg
    eggParameters.h
    eggPatch.I eggPatch.h
    eggPoint.I eggPoint.h eggPolygon.Ig
    eggPolygon.h eggPolysetMaker.h eggPoolUniquifier.h
    eggPrimitive.I eggPrimitive.h
    eggRenderMode.I eggRenderMode.hg
    eggSAnimData.I eggSAnimData.h eggSurface.I eggSurface.hg
    eggSwitchCondition.h eggTable.I eggTable.h eggTexture.Ig
    eggTexture.h eggTextureCollection.I eggTextureCollection.hg
    eggTriangleFan.I eggTriangleFan.h
    eggTriangleStrip.I eggTriangleStrip.h
    eggTransform.I eggTransform.h
    eggUserData.I eggUserData.h
    eggUtilities.I eggUtilities.h
    eggVertex.I eggVertex.h
    eggVertexAux.I eggVertexAux.h
    eggVertexPool.I eggVertexPool.h
    eggVertexUV.I eggVertexUV.h
    eggXfmAnimData.Ig
    eggXfmAnimData.h eggXfmSAnim.I eggXfmSAnim.h parserDefs.hg
    parser.yxx lexerDefs.h lexer.lxx pt_EggMaterial.hg
    vector_PT_EggMaterial.h pt_EggTexture.hg
    vector_PT_EggTexture.h pt_EggVertex.h vector_PT_EggVertex.h
  )

  set(P3EGG_SOURCES
    config_egg.cxx eggAnimData.cxx
    eggAnimPreload.cxx
    eggAttributes.cxx eggBin.cxxg
    eggBinMaker.cxx eggComment.cxx
    eggCompositePrimitive.cxx
    eggCoordinateSystem.cxxg
    eggCurve.cxx eggData.cxx eggExternalReference.cxxg
    eggFilenameNode.cxx eggGroup.cxx eggGroupNode.cxxg
    eggGroupUniquifier.cxx eggLine.cxx eggMaterial.cxxg
    eggMaterialCollection.cxx
    eggMesher.cxx
    eggMesherEdge.cxx
    eggMesherFanMaker.cxx
    eggMesherStrip.cxx
    eggMiscFuncs.cxx eggMorphList.cxxg
    eggNamedObject.cxx eggNameUniquifier.cxx eggNode.cxxg
    eggNurbsCurve.cxx eggNurbsSurface.cxx eggObject.cxxg
    eggParameters.cxx
    eggPatch.cxx
    eggPoint.cxx eggPolygon.cxx eggPolysetMaker.cxxg
    eggPoolUniquifier.cxx eggPrimitive.cxx eggRenderMode.cxxg
    eggSAnimData.cxx eggSurface.cxx eggSwitchCondition.cxxg
    eggTable.cxx eggTexture.cxx eggTextureCollection.cxxg
    eggTransform.cxx
    eggTriangleFan.cxx
    eggTriangleStrip.cxx
    eggUserData.cxx
    eggUtilities.cxx eggVertex.cxx
    eggVertexAux.cxx
    eggVertexPool.cxx eggVertexUV.cxx
    eggXfmAnimData.cxx eggXfmSAnim.cxx pt_EggMaterial.cxxg
    vector_PT_EggMaterial.cxx pt_EggTexture.cxxg
    vector_PT_EggTexture.cxx pt_EggVertex.cxxg
    vector_PT_EggVertex.cxxg
    lexer.cxx
    parser.cxx
  )

  composite_sources(p3egg P3EGG_SOURCES)
  add_library(p3egg ${P3EGG_HEADERS} ${P3EGG_SOURCES})
  target_link_libraries(p3egg p3prc p3pandabase p3express p3linmath p3mathutil)
  target_interrogate(p3egg ALL)
endif()
