#define BUILD_DIRECTORY $[and $[HAVE_GTK],$[HAVE_NET]]
#define USE_PACKAGES net gtk

#begin bin_target
  // We rename TARGET to gtk-pstats on Windows, so it won't compete
  // with Windows-native pstats.
  #define TARGET $[if $[WINDOWS_PLATFORM],gtk-pstats,pstats]
  #define LOCAL_LIBS \
    progbase pstatserver
  #define OTHER_LIBS \
    pstatclient:c linmath:c putil:c net:c express:c pandaexpress:m panda:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m \
    pystub

  #define SOURCES \
    gtkStats.cxx \
    gtkStatsGraph.cxx gtkStatsGraph.h \
    gtkStatsLabel.cxx gtkStatsLabel.h \
    gtkStatsLabelStack.cxx gtkStatsLabelStack.h \
    gtkStatsMenuId.h \
    gtkStatsMonitor.cxx gtkStatsMonitor.h gtkStatsMonitor.I \
    gtkStatsServer.cxx gtkStatsServer.h \
    gtkStatsStripChart.cxx gtkStatsStripChart.h

//     gtkStatsChartMenu.cxx gtkStatsChartMenu.h \
//     gtkStatsPianoRoll.cxx gtkStatsPianoRoll.h \

  #if $[DEVELOP_GTKSTATS]
    #define EXTRA_CDEFS $[EXTRA_CDEFS] DEVELOP_GTKSTATS
  #endif

#end bin_target

