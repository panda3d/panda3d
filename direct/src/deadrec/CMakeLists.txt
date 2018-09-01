set(P3DEADREC_HEADERS
    config_deadrec.h
    smoothMover.h smoothMover.I)

set(P3DEADREC_SOURCES
    config_deadrec.cxx
    smoothMover.cxx)

add_component_library(p3deadrec SYMBOL BUILDING_DIRECT_DEADREC
  ${P3DEADREC_HEADERS} ${P3DEADREC_SOURCES})
target_link_libraries(p3deadrec p3directbase panda)
target_interrogate(p3deadrec ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3deadrec DESTINATION lib RUNTIME DESTINATION bin)
endif()
install(FILES ${P3DEADREC_HEADERS} DESTINATION include/panda3d)
