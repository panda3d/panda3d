if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT HAVE_EGG)
  return()
endif()

set(P3EGG_QTESS_HEADERS
  config_egg_qtess.h
  eggQtess.h
  isoPlacer.h isoPlacer.I
  qtessGlobals.h
  qtessInputEntry.h qtessInputEntry.I
  qtessInputFile.h qtessInputFile.I
  qtessSurface.h qtessSurface.I
  subdivSegment.h subdivSegment.I
)

set(P3EGG_QTESS_SOURCES
  config_egg_qtess.cxx
  eggQtess.cxx
  isoPlacer.cxx
  qtessGlobals.cxx
  qtessInputEntry.cxx
  qtessInputFile.cxx
  qtessSurface.cxx
  subdivSegment.cxx
)

composite_sources(egg-qtess P3EGG_QTESS_SOURCES)
add_executable(egg-qtess ${P3EGG_QTESS_HEADERS} ${P3EGG_QTESS_SOURCES})
target_link_libraries(egg-qtess p3eggbase)

install(TARGETS egg-qtess EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
