add_library(p3showbase showBase.cxx showBase.h)
target_link_libraries(p3showbase p3directbase p3express p3event p3chan p3display p3dconfig p3prc)
target_interrogate(p3showbase ALL)

install(TARGETS p3showbase DESTINATION lib)
