if(NOT HAVE_MAC3 OR NOT HAVE_NET)
  return()
endif()

set(MACSTATS_HEADERS
  macStats.h
  macStatsAppDelegate.h
  macStatsChartMenu.h
  macStatsFlameGraph.h
  macStatsGraph.h
  macStatsLabel.h
  macStatsLabelStack.h
  macStatsMonitor.h
  macStatsPianoRoll.h
  macStatsServer.h
  macStatsStripChart.h
)

set(MACSTATS_SOURCES
  macStats.mm
  macStatsAppDelegate.mm
  macStatsChartMenu.mm
  macStatsFlameGraph.mm
  macStatsGraph.mm
  macStatsLabel.mm
  macStatsLabelStack.mm
  macStatsMonitor.mm
  macStatsPianoRoll.mm
  macStatsServer.mm
  macStatsStripChart.mm
  macStatsTimeline.mm
)

composite_sources(mac-stats MACSTATS_SOURCES)
add_executable(mac-stats ${MACSTATS_HEADERS} ${MACSTATS_SOURCES})
target_link_libraries(mac-stats p3progbase p3pstatserver PKG::MAC3)
set_target_properties(mac-stats PROPERTIES OUTPUT_NAME "pstats")

install(TARGETS mac-stats EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
