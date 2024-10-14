if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT APPLE OR NOT HAVE_NET)
  return()
endif()

set(MACSTATS_HEADERS
  macStats.h
  macStatsAppDelegate.h
  macStatsChartMenu.h
  macStatsChartMenuDelegate.h
  macStatsFlameGraph.h
  macStatsGraph.h
  macStatsGraphView.h
  macStatsGraphViewController.h
  macStatsLabel.h
  macStatsLabelStack.h
  macStatsMonitor.h
  macStatsPianoRoll.h
  macStatsScaleArea.h
  macStatsServer.h
  macStatsStripChart.h
  macStatsTimeline.h
)

set(MACSTATS_SOURCES
  macStats.mm
  macStatsAppDelegate.mm
  macStatsChartMenu.mm
  macStatsChartMenuDelegate.mm
  macStatsFlameGraph.mm
  macStatsGraph.mm
  macStatsGraphView.mm
  macStatsGraphViewController.mm
  macStatsLabel.mm
  macStatsLabelStack.mm
  macStatsMonitor.mm
  macStatsPianoRoll.mm
  macStatsScaleArea.mm
  macStatsServer.mm
  macStatsStripChart.mm
  macStatsTimeline.mm
)

composite_sources(mac-stats MACSTATS_SOURCES)
add_executable(mac-stats ${MACSTATS_HEADERS} ${MACSTATS_SOURCES})
target_link_libraries(mac-stats p3progbase p3pstatserver)
target_link_libraries(mac-stats "-framework Cocoa")
target_link_libraries(mac-stats "-framework Carbon")
target_link_libraries(mac-stats "-framework Quartz")

# This program is NOT actually called win-stats. It's just pstats
set_target_properties(mac-stats PROPERTIES OUTPUT_NAME "pstats")

install(TARGETS mac-stats EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
