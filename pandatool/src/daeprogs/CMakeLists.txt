if(NOT BUILD_TOOLS)
  return()
endif()

if(HAVE_EGG AND HAVE_FCOLLADA)

  add_executable(dae2egg daeToEgg.cxx daeToEgg.h)
  target_link_libraries(dae2egg p3daeegg p3eggbase p3progbase)
  install(TARGETS dae2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
