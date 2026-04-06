set(P3MOTIONTRAIL_HEADERS
  config_motiontrail.h
  cMotionTrail.h
)

set(P3MOTIONTRAIL_SOURCES
  config_motiontrail.cxx
  cMotionTrail.cxx
)

add_component_library(p3motiontrail SYMBOL BUILDING_DIRECT_MOTIONTRAIL
  ${P3MOTIONTRAIL_HEADERS} ${P3MOTIONTRAIL_SOURCES})
target_link_libraries(p3motiontrail p3directbase panda)
target_interrogate(p3motiontrail ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3motiontrail
    EXPORT Direct COMPONENT Direct
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT DirectDevel)
endif()
install(FILES ${P3MOTIONTRAIL_HEADERS} COMPONENT DirectDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
