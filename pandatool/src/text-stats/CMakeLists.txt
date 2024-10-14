if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT HAVE_NET)
  return()
endif()

set(TEXTSTATS_HEADERS
  textMonitor.h textMonitor.I
  textStats.h
)

set(TEXTSTATS_SOURCES
  textMonitor.cxx
  textStats.cxx
)

composite_sources(text-stats TEXTSTATS_SOURCES)
add_executable(text-stats ${TEXTSTATS_HEADERS} ${TEXTSTATS_SOURCES})
target_link_libraries(text-stats p3progbase p3pstatserver)

install(TARGETS text-stats EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
