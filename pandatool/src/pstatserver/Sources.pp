#define BUILD_DIRECTORY $[HAVE_NET]

#begin ss_lib_target
  #define TARGET pstatserver
  #define LOCAL_LIBS pandatoolbase
  #define OTHER_LIBS \
    pstatclient:c downloader:c net:c putil:c pipeline:c \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m

  #define SOURCES \
    pStatClientData.cxx pStatClientData.h pStatGraph.I pStatGraph.cxx \
    pStatGraph.h pStatListener.cxx pStatListener.h pStatMonitor.I \
    pStatMonitor.cxx pStatMonitor.h pStatPianoRoll.I pStatPianoRoll.cxx \
    pStatPianoRoll.h pStatReader.cxx pStatReader.h pStatServer.cxx \
    pStatServer.h pStatStripChart.I pStatStripChart.cxx \
    pStatStripChart.h pStatThreadData.I pStatThreadData.cxx \
    pStatThreadData.h pStatView.I pStatView.cxx pStatView.h \
    pStatViewLevel.I pStatViewLevel.cxx pStatViewLevel.h

  #define INSTALL_HEADERS \
    pStatClientData.h pStatGraph.I pStatGraph.h pStatListener.h \
    pStatMonitor.I pStatMonitor.h pStatPianoRoll.I pStatPianoRoll.h \
    pStatReader.h pStatServer.h pStatStripChart.I pStatStripChart.h \
    pStatThreadData.I pStatThreadData.h pStatView.I pStatView.h \
    pStatViewLevel.I pStatViewLevel.h

#end ss_lib_target

