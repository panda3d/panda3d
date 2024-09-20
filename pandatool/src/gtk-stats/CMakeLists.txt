if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT HAVE_GTK3 OR NOT HAVE_NET)
  return()
endif()

set(GTKSTATS_HEADERS
  gtkStatsChartMenu.h
  gtkStatsFlameGraph.h
  gtkStatsGraph.h
  gtkStatsLabel.h
  gtkStatsLabelStack.h
  gtkStatsMonitor.h gtkStatsMonitor.I
  gtkStatsPianoRoll.h
  gtkStatsServer.h
  gtkStatsStripChart.h
)

set(GTKSTATS_SOURCES
  gtkStats.cxx
  gtkStatsChartMenu.cxx
  gtkStatsFlameGraph.cxx
  gtkStatsGraph.cxx
  gtkStatsLabel.cxx
  gtkStatsLabelStack.cxx
  gtkStatsMonitor.cxx
  gtkStatsPianoRoll.cxx
  gtkStatsServer.cxx
  gtkStatsStripChart.cxx
  gtkStatsTimeline.cxx
)

composite_sources(gtk-stats GTKSTATS_SOURCES)
add_executable(gtk-stats ${GTKSTATS_HEADERS} ${GTKSTATS_SOURCES})
target_link_libraries(gtk-stats p3progbase p3pstatserver PKG::GTK3)

# This program is NOT actually called gtk-stats. It's pstats-gtk on Win32 and
# pstats everywhere else (as the Win32 GUI is not built).
if(WIN32 OR APPLE)
  set_target_properties(gtk-stats PROPERTIES OUTPUT_NAME "pstats-gtk")
else()
  set_target_properties(gtk-stats PROPERTIES OUTPUT_NAME "pstats")
endif()

install(TARGETS gtk-stats EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
