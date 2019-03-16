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

composite_sources(p3distort P3DISTORT_SOURCES)
add_component_library(p3distort SYMBOL BUILDING_PANDAFX
  ${P3DISTORT_HEADERS} ${P3DISTORT_SOURCES})
target_link_libraries(p3distort p3pgraphnodes p3display)
target_interrogate(p3distort ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3distort
    EXPORT Core COMPONENT Core
    DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DISTORT_HEADERS} COMPONENT CoreDevel DESTINATION include/panda3d)
