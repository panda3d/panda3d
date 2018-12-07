set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "CoreDevel")
add_metalib(p3dtool INIT init_libdtool dtool.h COMPONENTS p3dtoolutil p3dtoolbase)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS p3dtool
  EXPORT Core COMPONENT Core
  DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE COMPONENT CoreDevel)
