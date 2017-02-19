if(HAVE_EGG)
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
  add_library(p3egg2pg ${P3EGG2PG_HEADERS} ${P3EGG2PG_SOURCES})
  target_link_libraries(p3egg2pg p3parametrics p3collide p3egg p3pgraph p3chan p3char)
  target_interrogate(p3egg2pg ALL)

  install(TARGETS p3egg2pg DESTINATION lib)
  install(FILES ${P3EGG2PG_HEADERS} DESTINATION include/panda3d)
endif()
