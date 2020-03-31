if(NOT HAVE_EGG)
  return()
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "EggDevel")
add_metalib(pandaegg
  INIT init_libpandaegg pandaegg.h
  COMPONENTS p3egg p3egg2pg)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)
target_link_libraries(pandaegg panda)

install(TARGETS pandaegg
  EXPORT Egg COMPONENT Egg
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT EggDevel)

export_targets(Egg COMPONENT EggDevel)
