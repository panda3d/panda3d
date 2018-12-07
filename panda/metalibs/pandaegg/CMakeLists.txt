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
  DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE COMPONENT EggDevel)
