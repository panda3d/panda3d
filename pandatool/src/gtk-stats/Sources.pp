#define DIRECTORY_IF_GTKMM yes
#define USE_GTKMM yes
#define DIRECTORY_IF_NET yes
#define USE_NET yes

#begin bin_target

#if $[or $[<= $[OPTIMIZE],3], $[ne $[DO_PSTATS],]]
  #define TARGET gtk-stats
  #define LOCAL_LIBS \
    gtkbase progbase pstatserver
  #define OTHER_LIBS \
    pstatclient:c linmath:c putil:c express:c panda:m \
    dtoolutil:c dconfig:c dtoolconfig:m dtool:m
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
#else
  #define TARGET
  #define SOURCES
  #define INSTALL_HEADERS  
#endif  	
	

#end bin_target

