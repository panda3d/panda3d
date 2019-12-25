set(P3GRUTIL_HEADERS
  pipeOcclusionCullTraverser.I pipeOcclusionCullTraverser.h
  cardMaker.I cardMaker.h
  config_grutil.h
  movieTexture.I movieTexture.h
  fisheyeMaker.I fisheyeMaker.h
  frameRateMeter.I frameRateMeter.h
  meshDrawer.I meshDrawer.h
  meshDrawer2D.I meshDrawer2D.h
  geoMipTerrain.I geoMipTerrain.h
  sceneGraphAnalyzerMeter.I sceneGraphAnalyzerMeter.h
  heightfieldTesselator.I heightfieldTesselator.h
  shaderTerrainMesh.I shaderTerrainMesh.h
  lineSegs.I lineSegs.h
  multitexReducer.I multitexReducer.h
  nodeVertexTransform.I nodeVertexTransform.h
  pfmVizzer.I pfmVizzer.h
  rigidBodyCombiner.I rigidBodyCombiner.h
)

set(P3GRUTIL_SOURCES
  cardMaker.cxx
  movieTexture.cxx
  fisheyeMaker.cxx
  config_grutil.cxx
  frameRateMeter.cxx
  meshDrawer.cxx
  meshDrawer2D.cxx
  geoMipTerrain.cxx
  sceneGraphAnalyzerMeter.cxx
  heightfieldTesselator.cxx
  shaderTerrainMesh.cxx
  multitexReducer.cxx
  nodeVertexTransform.cxx
  pfmVizzer.cxx
  pipeOcclusionCullTraverser.cxx
  lineSegs.cxx
  rigidBodyCombiner.cxx
)

# This is a large file; let's build it separately
set_source_files_properties(multitexReducer.cxx PROPERTIES SKIP_UNITY_BUILD_INCLUSION YES)

composite_sources(p3grutil P3GRUTIL_SOURCES)
add_component_library(p3grutil SYMBOL BUILDING_PANDA_GRUTIL
  ${P3GRUTIL_HEADERS} ${P3GRUTIL_SOURCES})
target_link_libraries(p3grutil p3display p3text p3movies)
if(HAVE_AUDIO)
  target_link_libraries(p3grutil p3audio)
endif()
target_interrogate(p3grutil ALL)

if(HAVE_EIGEN)
  # Eigen puts this over the threshold where bigobj is needed on MSVC
  set_target_properties(p3grutil PROPERTIES MSVC_BIGOBJ ON)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3grutil
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3GRUTIL_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
