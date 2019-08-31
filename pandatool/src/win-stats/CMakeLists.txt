if(NOT WIN32 OR NOT HAVE_NET)
  return()
endif()

set(WINSTATS_HEADERS
  winStatsChartMenu.h
  winStatsGraph.h
  winStats.h
  winStatsLabel.h
  winStatsLabelStack.h
  winStatsMenuId.h
  winStatsMonitor.h winStatsMonitor.I
  winStatsPianoRoll.h
  winStatsServer.h
  winStatsStripChart.h
)

set(WINSTATS_SOURCES
  winStatsChartMenu.cxx
  winStats.cxx
  winStatsGraph.cxx
  winStatsLabel.cxx
  winStatsLabelStack.cxx
  winStatsMonitor.cxx
  winStatsPianoRoll.cxx
  winStatsServer.cxx
  winStatsStripChart.cxx
)

composite_sources(win-stats WINSTATS_SOURCES)
add_executable(win-stats ${WINSTATS_HEADERS} ${WINSTATS_SOURCES})
target_link_libraries(win-stats p3progbase p3pstatserver)

# This program is NOT actually called win-stats. It's just pstats.exe
set_target_properties(win-stats PROPERTIES OUTPUT_NAME "pstats")

install(TARGETS win-stats EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
