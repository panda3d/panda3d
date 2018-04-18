add_component_library(p3showbase SYMBOL BUILDING_DIRECT_SHOWBASE
  showBase.cxx showBase.h)
target_link_libraries(p3showbase p3directbase panda)
target_interrogate(p3showbase ALL)

install(TARGETS p3showbase DESTINATION lib)
