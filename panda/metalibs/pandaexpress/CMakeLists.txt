add_metalib(pandaexpress
  INIT init_libpandaexpress pandaexpress.h
  COMPONENTS p3downloader p3express)

install(TARGETS pandaexpress DESTINATION lib RUNTIME DESTINATION bin)
