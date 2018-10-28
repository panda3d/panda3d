if(NOT HAVE_NET)
  return()
endif()

set(P3PSTATSERVER_HEADERS
  pStatClientData.h
  pStatGraph.h pStatGraph.I
  pStatListener.h
  pStatMonitor.h pStatMonitor.I
  pStatPianoRoll.h pStatPianoRoll.I
  pStatReader.h
  pStatServer.h
  pStatStripChart.h pStatStripChart.I
  pStatThreadData.h pStatThreadData.I
  pStatView.h pStatView.I
  pStatViewLevel.h pStatViewLevel.I
)

set(P3PSTATSERVER_SOURCES
  pStatClientData.cxx pStatGraph.cxx
  pStatListener.cxx
  pStatMonitor.cxx pStatPianoRoll.cxx
  pStatReader.cxx pStatServer.cxx
  pStatStripChart.cxx pStatThreadData.cxx
  pStatView.cxx pStatViewLevel.cxx
)

composite_sources(p3pstatserver P3PSTATSERVER_SOURCES)
add_library(p3pstatserver STATIC ${P3PSTATSERVER_HEADERS} ${P3PSTATSERVER_SOURCES})
target_link_libraries(p3pstatserver p3pandatoolbase panda)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
