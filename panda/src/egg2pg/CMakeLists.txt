if(NOT HAVE_EGG)
  return()
endif()

set(P3EGG2PG_HEADERS
  animBundleMaker.h
  characterMaker.h
  config_egg2pg.h
  deferredNodeProperty.h
  eggBinner.h
  eggLoader.h eggLoader.I
  eggRenderState.h eggRenderState.I
  eggSaver.h eggSaver.I
  egg_parametrics.h
  load_egg_file.h
  save_egg_file.h
  loaderFileTypeEgg.h
)

set(P3EGG2PG_SOURCES
  animBundleMaker.cxx
  characterMaker.cxx
  config_egg2pg.cxx
  deferredNodeProperty.cxx
  eggBinner.cxx
  eggLoader.cxx
  eggRenderState.cxx
  eggSaver.cxx
  egg_parametrics.cxx
  load_egg_file.cxx
  save_egg_file.cxx
  loaderFileTypeEgg.cxx
)

composite_sources(p3egg2pg P3EGG2PG_SOURCES)
add_component_library(p3egg2pg SYMBOL BUILDING_PANDA_EGG2PG
  ${P3EGG2PG_HEADERS} ${P3EGG2PG_SOURCES})
target_link_libraries(p3egg2pg panda p3egg)
target_interrogate(p3egg2pg ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3egg2pg
    EXPORT Egg COMPONENT Egg
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT EggDevel)
endif()
install(FILES ${P3EGG2PG_HEADERS} COMPONENT EggDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
