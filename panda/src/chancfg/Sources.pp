#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET chancfg
  #define LOCAL_LIBS \
    putil display sgattrib linmath graph sgraph gobj display gsgbase \
    mathutil

  #define SOURCES \
    chancfg.I chancfg.N chancfg.cxx chancfg.h chanlayout.I \
    chanlayout.cxx chanlayout.h chanparse.I chanparse.cxx chanparse.h \
    chansetup.I chansetup.cxx chansetup.h chanwindow.I chanwindow.cxx \
    chanwindow.h

  #define INSTALL_HEADERS \
    chancfg.I chancfg.h chanlayout.I chanlayout.h chansetup.I \
    chansetup.h chanshare.h chanviewport.I chanviewport.h chanwindow.I \
    chanwindow.h

  #define INSTALL_CONFIG \
    layout_db setup_db window_db

  #define PRECOMPILED_HEADER chancfg_headers.h 

  #define IGATESCAN chancfg.h

#end lib_target

#begin test_bin_target
  #define TARGET test_chancfg
  #define LOCAL_LIBS \
    chancfg putil

  #define SOURCES \
    test_chancfg.cxx

#end test_bin_target

