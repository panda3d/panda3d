add_library(pandaphysics pandaphysics.cxx)
target_link_libraries(pandaphysics p3physics p3particlesystem)

install(TARGETS pandaphysics DESTINATION lib)
