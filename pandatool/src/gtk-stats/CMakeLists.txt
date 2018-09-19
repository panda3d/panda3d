if(NOT HAVE_GTK2 OR NOT HAVE_NET)
  return()
endif()

set(GTKSTATS_HEADERS
    gtkStatsChartMenu.h
    gtkStatsGraph.h
    gtkStatsLabel.h
    gtkStatsLabelStack.h
    gtkStatsMenuId.h
    gtkStatsMonitor.h gtkStatsMonitor.I
    gtkStatsPianoRoll.h
    gtkStatsServer.h
    gtkStatsStripChart.h)

set(GTKSTATS_SOURCES
    gtkStats.cxx
    gtkStatsChartMenu.cxx
    gtkStatsGraph.cxx
    gtkStatsLabel.cxx
    gtkStatsLabelStack.cxx
    gtkStatsMonitor.cxx
    gtkStatsPianoRoll.cxx
    gtkStatsServer.cxx
    gtkStatsStripChart.cxx)

composite_sources(gtkstats GTKSTATS_SOURCES)
add_executable(gtkstats ${GTKSTATS_HEADERS} ${GTKSTATS_SOURCES})
target_link_libraries(gtkstats p3progbase p3pstatserver p3pystub PKG::GTK2)

# This program is NOT actually called gtkstats. It's pstats-gtk on Win32 and
# pstats everywhere else (as the Win32 GUI is not built).
if(WIN32)
  set_target_properties(gtkstats PROPERTIES OUTPUT_NAME "pstats-gtk")
else()
  set_target_properties(gtkstats PROPERTIES OUTPUT_NAME "pstats")
endif()

install(TARGETS gtkstats DESTINATION bin)
