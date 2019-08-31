if(NOT HAVE_EGG)
  return()
endif()

set(P3EGG_HEADERS
  config_egg.h eggAnimData.I eggAnimData.h
  eggAnimPreload.I eggAnimPreload.h
  eggAttributes.I
  eggAttributes.h eggBin.h eggBinMaker.h eggComment.I
  eggComment.h
  eggCompositePrimitive.I eggCompositePrimitive.h
  eggCoordinateSystem.I eggCoordinateSystem.h
  eggCurve.I eggCurve.h eggData.I eggData.h
  eggExternalReference.I eggExternalReference.h
  eggFilenameNode.I eggFilenameNode.h eggGroup.I eggGroup.h
  eggGroupNode.I eggGroupNode.h eggGroupUniquifier.h
  eggLine.I eggLine.h
  eggMaterial.I eggMaterial.h eggMaterialCollection.I
  eggMaterialCollection.h
  eggMesher.h eggMesher.I
  eggMesherEdge.h eggMesherEdge.I
  eggMesherFanMaker.h eggMesherFanMaker.I
  eggMesherStrip.h eggMesherStrip.I
  eggMiscFuncs.I eggMiscFuncs.h
  eggMorph.I eggMorph.h eggMorphList.I eggMorphList.h
  eggNamedObject.I eggNamedObject.h eggNameUniquifier.h
  eggNode.I eggNode.h eggNurbsCurve.I eggNurbsCurve.h
  eggNurbsSurface.I eggNurbsSurface.h eggObject.I eggObject.h
  eggParameters.h
  eggPatch.I eggPatch.h
  eggPoint.I eggPoint.h eggPolygon.I
  eggPolygon.h eggPolysetMaker.h eggPoolUniquifier.h
  eggPrimitive.I eggPrimitive.h
  eggRenderMode.I eggRenderMode.h
  eggSAnimData.I eggSAnimData.h eggSurface.I eggSurface.h
  eggSwitchCondition.h eggTable.I eggTable.h eggTexture.I
  eggTexture.h eggTextureCollection.I eggTextureCollection.h
  eggTriangleFan.I eggTriangleFan.h
  eggTriangleStrip.I eggTriangleStrip.h
  eggTransform.I eggTransform.h
  eggUserData.I eggUserData.h
  eggUtilities.I eggUtilities.h
  eggVertex.I eggVertex.h
  eggVertexAux.I eggVertexAux.h
  eggVertexPool.I eggVertexPool.h
  eggVertexUV.I eggVertexUV.h
  eggXfmAnimData.I
  eggXfmAnimData.h eggXfmSAnim.I eggXfmSAnim.h parserDefs.h
  parser.yxx lexerDefs.h lexer.lxx pt_EggMaterial.h
  vector_PT_EggMaterial.h pt_EggTexture.h
  vector_PT_EggTexture.h pt_EggVertex.h vector_PT_EggVertex.h
)

set(P3EGG_SOURCES
  config_egg.cxx eggAnimData.cxx
  eggAnimPreload.cxx
  eggAttributes.cxx eggBin.cxx
  eggBinMaker.cxx eggComment.cxx
  eggCompositePrimitive.cxx
  eggCoordinateSystem.cxx
  eggCurve.cxx eggData.cxx eggExternalReference.cxx
  eggFilenameNode.cxx eggGroup.cxx eggGroupNode.cxx
  eggGroupUniquifier.cxx eggLine.cxx eggMaterial.cxx
  eggMaterialCollection.cxx
  eggMesher.cxx
  eggMesherEdge.cxx
  eggMesherFanMaker.cxx
  eggMesherStrip.cxx
  eggMiscFuncs.cxx eggMorphList.cxx
  eggNamedObject.cxx eggNameUniquifier.cxx eggNode.cxx
  eggNurbsCurve.cxx eggNurbsSurface.cxx eggObject.cxx
  eggParameters.cxx
  eggPatch.cxx
  eggPoint.cxx eggPolygon.cxx eggPolysetMaker.cxx
  eggPoolUniquifier.cxx eggPrimitive.cxx eggRenderMode.cxx
  eggSAnimData.cxx eggSurface.cxx eggSwitchCondition.cxx
  eggTable.cxx eggTexture.cxx eggTextureCollection.cxx
  eggTransform.cxx
  eggTriangleFan.cxx
  eggTriangleStrip.cxx
  eggUserData.cxx
  eggUtilities.cxx eggVertex.cxx
  eggVertexAux.cxx
  eggVertexPool.cxx eggVertexUV.cxx
  eggXfmAnimData.cxx eggXfmSAnim.cxx pt_EggMaterial.cxx
  vector_PT_EggMaterial.cxx pt_EggTexture.cxx
  vector_PT_EggTexture.cxx pt_EggVertex.cxx
  vector_PT_EggVertex.cxx
)

set(P3EGG_IGATEEXT
  eggGroupNode_ext.cxx
  eggGroupNode_ext.h
)

# These cannot be interrogated, and are excluded from the composites.
add_bison_target(parser.cxx parser.yxx DEFINES parser.h PREFIX eggyy)
add_flex_target(lexer.cxx lexer.lxx CASE_INSENSITIVE PREFIX eggyy)
set(P3EGG_PARSER_SOURCES parser.cxx lexer.cxx)

composite_sources(p3egg P3EGG_SOURCES)
add_component_library(p3egg SYMBOL BUILDING_PANDA_EGG
  ${P3EGG_HEADERS} ${P3EGG_SOURCES} ${P3EGG_PARSER_SOURCES})
target_link_libraries(p3egg panda)
target_interrogate(p3egg ${P3EGG_HEADERS} ${P3EGG_SOURCES} EXTENSIONS ${P3EGG_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3egg
    EXPORT Egg COMPONENT Egg
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT EggDevel)
endif()
install(FILES ${P3EGG_HEADERS} COMPONENT EggDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
