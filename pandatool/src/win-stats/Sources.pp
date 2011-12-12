#define BUILD_DIRECTORY $[and $[WINDOWS_PLATFORM],$[HAVE_NET]]
#define USE_PACKAGES net

#begin bin_target
  #define TARGET pstats
  #define LOCAL_LIBS \
    p3progbase p3pstatserver
  #define OTHER_LIBS \
    p3pstatclient:c p3linmath:c p3putil:c p3net:c p3express:c pandaexpress:m panda:m \
    p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m \
    p3pystub

  #define SOURCES \
    winStats.cxx \
    winStatsChartMenu.cxx winStatsChartMenu.h \
    winStatsGraph.cxx winStatsGraph.h \
    winStatsLabel.cxx winStatsLabel.h \
    winStatsLabelStack.cxx winStatsLabelStack.h \
    winStatsMenuId.h \
    winStatsMonitor.cxx winStatsMonitor.h winStatsMonitor.I \
    winStatsPianoRoll.cxx winStatsPianoRoll.h \
    winStatsServer.cxx winStatsServer.h \
    winStatsStripChart.cxx winStatsStripChart.h

  #if $[DEVELOP_WINSTATS]
    #define EXTRA_CDEFS $[EXTRA_CDEFS] DEVELOP_WINSTATS
  #endif
  #define EXTRA_CDEFS $[EXTRA_CDEFS] WIN32_LEAN_AND_MEAN

  #define WIN_SYS_LIBS Imm32.lib winmm.lib kernel32.lib oldnames.lib user32.lib gdi32.lib

#end bin_target

