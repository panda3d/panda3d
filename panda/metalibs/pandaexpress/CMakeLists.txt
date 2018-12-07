set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "CoreDevel")
add_metalib(pandaexpress
  INIT init_libpandaexpress pandaexpress.h
  COMPONENTS p3downloader p3express)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandaexpress
  EXPORT Core COMPONENT Core
  DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE COMPONENT CoreDevel)
