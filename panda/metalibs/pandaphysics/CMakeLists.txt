add_metalib(pandaphysics
  INIT init_libpandaphysics pandaphysics.h
  COMPONENTS p3physics p3particlesystem)

install(TARGETS pandaphysics DESTINATION lib RUNTIME DESTINATION bin)
