set(P3DEADREC_HEADERS
    config_deadrec.h
    smoothMover.h smoothMover.I)

set(P3DEADREC_SOURCES
    config_deadrec.cxx
    smoothMover.cxx)

add_library(p3deadrec ${P3DEADREC_HEADERS} ${P3DEADREC_SOURCES})
target_link_libraries(p3deadrec p3directbase p3linmath p3pgraph p3prc)
target_interrogate(p3deadrec ALL)

install(TARGETS p3deadrec DESTINATION lib)
