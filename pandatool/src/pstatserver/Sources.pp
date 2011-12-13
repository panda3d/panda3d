#define BUILD_DIRECTORY $[HAVE_NET]

#begin ss_lib_target
  #define TARGET p3pstatserver
  #define LOCAL_LIBS p3pandatoolbase
  #define OTHER_LIBS \
    p3pstatclient:c p3downloader:c p3net:c p3putil:c p3pipeline:c \
    panda:m \
    p3pandabase:c p3express:c p3linmath:c pandaexpress:m \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m

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

