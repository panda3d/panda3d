set(P3DISTORT_HEADERS
  config_distort.h
  cylindricalLens.h cylindricalLens.I
  fisheyeLens.h fisheyeLens.I
  nonlinearImager.h nonlinearImager.I
  oSphereLens.h oSphereLens.I
  projectionScreen.h projectionScreen.I
  pSphereLens.h pSphereLens.I
)

set(P3DISTORT_SOURCES
  config_distort.cxx
  cylindricalLens.cxx
  fisheyeLens.cxx
  nonlinearImager.cxx
  oSphereLens.cxx
  projectionScreen.cxx
  pSphereLens.cxx
)

composite_sources(pandafx P3DISTORT_SOURCES)
add_library(pandafx ${P3DISTORT_HEADERS} ${P3DISTORT_SOURCES})
set_target_properties(pandafx PROPERTIES DEFINE_SYMBOL BUILDING_PANDAFX)
target_link_libraries(pandafx panda)
target_interrogate(pandafx ALL)

install(TARGETS pandafx
  EXPORT Core COMPONENT Core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT CoreDevel)
install(FILES ${P3DISTORT_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
