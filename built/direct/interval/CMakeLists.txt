set(P3INTERVAL_HEADERS
  config_interval.h
  cInterval.I cInterval.h
  cIntervalManager.I cIntervalManager.h
  cConstraintInterval.I cConstraintInterval.h
  cConstrainTransformInterval.I cConstrainTransformInterval.h
  cConstrainPosInterval.I cConstrainPosInterval.h
  cConstrainHprInterval.I cConstrainHprInterval.h
  cConstrainPosHprInterval.I cConstrainPosHprInterval.h
  cLerpInterval.I cLerpInterval.h
  cLerpNodePathInterval.I cLerpNodePathInterval.h
  cLerpAnimEffectInterval.I cLerpAnimEffectInterval.h
  cMetaInterval.I cMetaInterval.h
  hideInterval.I hideInterval.h
  lerpblend.h
  showInterval.I showInterval.h
  waitInterval.I waitInterval.h
  lerp_helpers.h
)

set(P3INTERVAL_SOURCES
  config_interval.cxx
  cInterval.cxx
  cIntervalManager.cxx
  cConstraintInterval.cxx
  cConstrainTransformInterval.cxx
  cConstrainPosInterval.cxx
  cConstrainHprInterval.cxx
  cConstrainPosHprInterval.cxx
  cLerpInterval.cxx
  cLerpNodePathInterval.cxx
  cLerpAnimEffectInterval.cxx
  cMetaInterval.cxx
  hideInterval.cxx
  lerpblend.cxx
  showInterval.cxx
  waitInterval.cxx
)

set(P3INTERVAL_IGATEEXT
  cInterval_ext.cxx
  cInterval_ext.h
)

composite_sources(p3interval P3INTERVAL_SOURCES)
add_component_library(p3interval SYMBOL BUILDING_DIRECT_INTERVAL
  ${P3INTERVAL_HEADERS} ${P3INTERVAL_SOURCES})
target_link_libraries(p3interval p3directbase panda)
target_interrogate(p3interval ALL EXTENSIONS ${P3INTERVAL_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3interval
    EXPORT Direct COMPONENT Direct
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT DirectDevel)
endif()
install(FILES ${P3INTERVAL_HEADERS} COMPONENT DirectDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
