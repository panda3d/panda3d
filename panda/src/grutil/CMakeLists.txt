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

composite_sources(p3grutil P3GRUTIL_SOURCES)
add_library(p3grutil ${P3GRUTIL_HEADERS} ${P3GRUTIL_SOURCES})
target_link_libraries(p3grutil p3display p3text p3movies)
if(HAVE_AUDIO)
  target_link_libraries(p3grutil p3audio)
endif()
target_interrogate(p3grutil ALL)

install(TARGETS p3grutil DESTINATION lib)
