if(HAVE_EGG)
  set(PANDAEGG_LINK_TARGETS p3egg p3egg2pg)

  add_library(pandaegg pandaegg.cxx)
  target_link_libraries(pandaegg ${PANDAEGG_LINK_TARGETS})
  target_link_libraries(pandaegg panda)

  install(TARGETS pandaegg DESTINATION lib)
endif()
