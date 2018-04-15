set(P3DEADREC_HEADERS
    config_deadrec.h
    smoothMover.h smoothMover.I)

set(P3DEADREC_SOURCES
    config_deadrec.cxx
    smoothMover.cxx)

add_library(p3deadrec ${P3DEADREC_HEADERS} ${P3DEADREC_SOURCES})
set_target_properties(p3deadrec PROPERTIES DEFINE_SYMBOL BUILDING_DIRECT_DEADREC)
target_link_libraries(p3deadrec p3directbase panda)
target_interrogate(p3deadrec ALL)

install(TARGETS p3deadrec DESTINATION lib)
install(FILES ${P3DEADREC_HEADERS} DESTINATION include/panda3d)
