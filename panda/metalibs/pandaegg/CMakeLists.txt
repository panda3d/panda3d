if(HAVE_EGG)
  set(PANDAEGG_LINK_TARGETS p3egg p3egg2pg panda)

  add_library(pandaegg pandaegg.cxx)
  target_link_libraries(pandaegg ${PANDAEGG_LINK_TARGETS})

  install(TARGETS pandaegg DESTINATION lib)
endif()
