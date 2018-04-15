add_library(p3showbase showBase.cxx showBase.h)
set_target_properties(p3showbase PROPERTIES DEFINE_SYMBOL BUILDING_DIRECT_SHOWBASE)
target_link_libraries(p3showbase p3directbase panda)
target_interrogate(p3showbase ALL)

install(TARGETS p3showbase DESTINATION lib)
