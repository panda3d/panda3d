if(HAVE_NET)
  set(P3PSTATSERVER_HEADERS
      pStatClientData.h pStatGraph.I
      pStatGraph.h pStatListener.h pStatMonitor.I
      pStatMonitor.h pStatPianoRoll.I
      pStatPianoRoll.h pStatReader.h
      pStatServer.h pStatStripChart.I
      pStatStripChart.h pStatThreadData.I
      pStatThreadData.h pStatView.I pStatView.h
      pStatViewLevel.I pStatViewLevel.h)

  set(P3PSTATSERVER_SOURCES
      pStatClientData.cxx pStatGraph.cxx
      pStatListener.cxx
      pStatMonitor.cxx pStatPianoRoll.cxx
      pStatReader.cxx pStatServer.cxx
      pStatStripChart.cxx pStatThreadData.cxx
      pStatView.cxx pStatViewLevel.cxx)

  composite_sources(p3pstatserver P3PSTATSERVER_SOURCES)
  add_library(p3pstatserver STATIC ${P3PSTATSERVER_HEADERS} ${P3PSTATSERVER_SOURCES})
  target_link_libraries(p3pstatserver p3pandatoolbase p3pstatclient p3linmath)

  # This is only needed for binaries in the pandatool package. It is not useful
  # for user applications, so it is not installed.
endif()
