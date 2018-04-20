if(NOT HAVE_EGG)
  return()
endif()

set(PANDAEGG_LINK_TARGETS p3egg p3egg2pg)

add_metalib(pandaegg pandaegg.cxx COMPONENTS ${PANDAEGG_LINK_TARGETS})
target_link_libraries(pandaegg panda)

install(TARGETS pandaegg DESTINATION lib)
