set(P3AI_HEADERS
  aiBehaviors.h
  aiCharacter.h
  aiGlobals.h
  aiNode.h
  aiPathFinder.h
  aiWorld.h
  arrival.h
  config_ai.h
  evade.h
  flee.h
  flock.h
  meshNode.h
  obstacleAvoidance.h
  pathFind.h
  pathFollow.h
  pursue.h
  seek.h
  wander.h
)

set(P3AI_SOURCES
  aiBehaviors.cxx
  aiCharacter.cxx
  aiNode.cxx
  aiPathFinder.cxx
  aiWorld.cxx
  arrival.cxx
  config_ai.cxx
  evade.cxx
  flee.cxx
  flock.cxx
  meshNode.cxx
  obstacleAvoidance.cxx
  pathFind.cxx
  pathFollow.cxx
  pursue.cxx
  seek.cxx
  wander.cxx
)

composite_sources(p3ai P3AI_SOURCES)
add_library(p3ai ${P3AI_HEADERS} ${P3AI_SOURCES})
set_target_properties(p3ai PROPERTIES DEFINE_SYMBOL BUILDING_PANDAAI)
target_link_libraries(p3ai p3contribbase)
target_interrogate(p3ai ALL)

install(TARGETS p3ai
  EXPORT Contrib COMPONENT Contrib
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT ContribDevel)
install(FILES ${P3AI_HEADERS} COMPONENT ContribDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
