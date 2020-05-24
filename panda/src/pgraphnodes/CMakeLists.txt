set(P3PGRAPHNODES_HEADERS
  ambientLight.h ambientLight.I
  callbackNode.h callbackNode.I
  computeNode.h computeNode.I
  config_pgraphnodes.h
  directionalLight.h directionalLight.I
  fadeLodNode.I fadeLodNode.h fadeLodNodeData.h
  lightLensNode.h lightLensNode.I
  lightNode.h lightNode.I
  lodNode.I lodNode.h lodNodeType.h
  nodeCullCallbackData.h nodeCullCallbackData.I
  pointLight.h pointLight.I
  rectangleLight.h rectangleLight.I
  sceneGraphAnalyzer.h sceneGraphAnalyzer.I
  selectiveChildNode.h selectiveChildNode.I
  sequenceNode.h sequenceNode.I
  shaderGenerator.h shaderGenerator.I
  sphereLight.h sphereLight.I
  spotlight.h spotlight.I
  switchNode.h switchNode.I
  uvScrollNode.I uvScrollNode.h
)

set(P3PGRAPHNODES_SOURCES
  ambientLight.cxx
  callbackNode.cxx
  computeNode.cxx
  config_pgraphnodes.cxx
  directionalLight.cxx
  fadeLodNode.cxx fadeLodNodeData.cxx
  lightLensNode.cxx
  lightNode.cxx
  lodNode.cxx lodNodeType.cxx
  nodeCullCallbackData.cxx
  pointLight.cxx
  rectangleLight.cxx
  sceneGraphAnalyzer.cxx
  selectiveChildNode.cxx
  sequenceNode.cxx
  shaderGenerator.cxx
  sphereLight.cxx
  spotlight.cxx
  switchNode.cxx
  uvScrollNode.cxx
)

composite_sources(p3pgraphnodes P3PGRAPHNODES_SOURCES)
add_component_library(p3pgraphnodes SYMBOL BUILDING_PANDA_PGRAPHNODES
  ${P3PGRAPHNODES_HEADERS} ${P3PGRAPHNODES_SOURCES})
target_link_libraries(p3pgraphnodes p3pgraph)
target_interrogate(p3pgraphnodes ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3pgraphnodes
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PGRAPHNODES_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
