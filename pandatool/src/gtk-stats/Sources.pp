#define BUILD_DIRECTORY $[and $[HAVE_GTK],$[HAVE_NET]]
#define USE_PACKAGES net gtk

#begin bin_target
  // We suspect gtk will not be built universal on OSX.  Don't try.
  #define UNIVERSAL_BINARIES

  // We rename TARGET to pstats-gtk on Windows, so it won't compete
  // with Windows-native pstats.
  #define TARGET $[if $[WINDOWS_PLATFORM],pstats-gtk,pstats]
  #define LOCAL_LIBS \
    p3progbase p3pstatserver
  #define OTHER_LIBS \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3pandabase:c p3pnmimage:c p3event:c p3pstatclient:c \
    p3linmath:c p3putil:c p3pipeline:c p3express:c pandaexpress:m panda:m \
    p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m \
    p3pystub

  #define SOURCES \
    gtkStats.cxx \
    gtkStatsChartMenu.cxx gtkStatsChartMenu.h \
    gtkStatsGraph.cxx gtkStatsGraph.h \
    gtkStatsLabel.cxx gtkStatsLabel.h \
    gtkStatsLabelStack.cxx gtkStatsLabelStack.h \
    gtkStatsMenuId.h \
    gtkStatsMonitor.cxx gtkStatsMonitor.h gtkStatsMonitor.I \
    gtkStatsPianoRoll.cxx gtkStatsPianoRoll.h \
    gtkStatsServer.cxx gtkStatsServer.h \
    gtkStatsStripChart.cxx gtkStatsStripChart.h

  #if $[DEVELOP_GTKSTATS]
    #define EXTRA_CDEFS $[EXTRA_CDEFS] DEVELOP_GTKSTATS
  #endif

#end bin_target

