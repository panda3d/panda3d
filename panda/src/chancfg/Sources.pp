#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET chancfg
  #define LOCAL_LIBS \
    putil display sgattrib linmath graph sgraph gobj display gsgbase \
    mathutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    chancfg.I chancfg.N chancfg.h chanlayout.I \
    chanlayout.h chanparse.I chanparse.h \
    chansetup.I chansetup.h chanwindow.I chanwindow.h
    
  #define INCLUDED_SOURCES \
    chancfg.cxx chanlayout.cxx chanparse.cxx  \
     chansetup.cxx chanwindow.cxx

  #define INSTALL_HEADERS \
    chancfg.I chancfg.h chanlayout.I chanlayout.h chansetup.I \
    chansetup.h chanshare.h chanviewport.I chanviewport.h chanwindow.I \
    chanwindow.h

  #define INSTALL_CONFIG \
    layout_db setup_db window_db

  #define IGATESCAN chancfg.h

#end lib_target

#begin test_bin_target
  #define TARGET test_chancfg
  #define LOCAL_LIBS \
    chancfg putil

  #define SOURCES \
    test_chancfg.cxx

#end test_bin_target

