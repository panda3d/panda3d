set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "CoreDevel")
add_metalib(pandaphysics
  INIT init_libpandaphysics pandaphysics.h
  COMPONENTS p3physics p3particlesystem)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandaphysics
  EXPORT Core COMPONENT Core
  DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE COMPONENT CoreDevel)
