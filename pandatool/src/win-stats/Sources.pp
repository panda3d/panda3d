#define BUILD_DIRECTORY $[and $[WINDOWS_PLATFORM],$[HAVE_NET],$[WANT_WINSTATS]]
#define USE_PACKAGES net

#begin bin_target
  #define TARGET pstats
  #define LOCAL_LIBS \
    progbase pstatserver
  #define OTHER_LIBS \
    pstatclient:c linmath:c putil:c net:c express:c pandaexpress:m panda:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m \
    pystub

  #define SOURCES \
    winStats.cxx \
    winStatsGraph.cxx winStatsGraph.h \
    winStatsLabel.cxx winStatsLabel.h \
    winStatsLabelStack.cxx winStatsLabelStack.h \
    winStatsServer.cxx winStatsServer.h \
    winStatsMonitor.cxx winStatsMonitor.h \
    winStatsStripChart.cxx winStatsStripChart.h

  #define WIN_SYS_LIBS Imm32.lib winmm.lib kernel32.lib oldnames.lib user32.lib gdi32.lib

#end bin_target

