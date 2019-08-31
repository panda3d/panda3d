set(P3DEADREC_HEADERS
  config_deadrec.h
  smoothMover.h smoothMover.I
)

set(P3DEADREC_SOURCES
  config_deadrec.cxx
  smoothMover.cxx
)

add_component_library(p3deadrec SYMBOL BUILDING_DIRECT_DEADREC
  ${P3DEADREC_HEADERS} ${P3DEADREC_SOURCES})
target_link_libraries(p3deadrec p3directbase panda)
target_interrogate(p3deadrec ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3deadrec
    EXPORT Direct COMPONENT Direct
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT DirectDevel)
endif()
install(FILES ${P3DEADREC_HEADERS} COMPONENT DirectDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
