#define DIRECTORY_IF_GTKMM yes
#define USE_GTKMM yes

#begin bin_target
  #define TARGET gtk-stats
  #define LOCAL_LIBS \
    gtkbase progbase pstatserver
  #define OTHER_LIBS \
    pstatclient:c linmath:c putil:c express:c panda:m \
    dtoolutil:c dconfig:c dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    gtkStats.cxx gtkStats.h gtkStatsGuide.cxx gtkStatsGuide.h \
    gtkStatsLabel.cxx gtkStatsLabel.h gtkStatsMainWindow.cxx \
    gtkStatsMainWindow.h gtkStatsMonitor.cxx gtkStatsMonitor.h \
    gtkStatsPianoRoll.I gtkStatsPianoRoll.cxx gtkStatsPianoRoll.h \
    gtkStatsPianoWindow.cxx gtkStatsPianoWindow.h gtkStatsServer.cxx \
    gtkStatsServer.h gtkStatsStripChart.I gtkStatsStripChart.cxx \
    gtkStatsStripChart.h gtkStatsStripWindow.cxx gtkStatsStripWindow.h \
    gtkStatsWindow.cxx gtkStatsWindow.h

#end bin_target

