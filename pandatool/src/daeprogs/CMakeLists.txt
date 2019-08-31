if(HAVE_EGG AND HAVE_FCOLLADA)

  add_executable(egg2dae eggToDAE.cxx eggToDAE.h)
  target_link_libraries(egg2dae p3daeegg p3eggbase p3progbase)
  install(TARGETS egg2dae EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

  add_executable(dae2egg daeToEgg.cxx daeToEgg.h)
  target_link_libraries(dae2egg p3daeegg p3eggbase p3progbase)
  install(TARGETS dae2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
